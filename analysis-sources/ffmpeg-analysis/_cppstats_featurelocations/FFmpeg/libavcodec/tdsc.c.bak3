


































#include <stdint.h>
#include <zlib.h>

#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define BITMAPINFOHEADER_SIZE 0x28
#define TDSF_HEADER_SIZE 0x56
#define TDSB_HEADER_SIZE 0x08

typedef struct TDSCContext {
AVCodecContext *jpeg_avctx; 

int width, height;
GetByteContext gbc;

AVFrame *refframe; 
AVFrame *jpgframe; 
uint8_t *tilebuffer; 


uint8_t *deflatebuffer;
uLongf deflatelen;


uint8_t *cursor;
int cursor_stride;
int cursor_w, cursor_h, cursor_x, cursor_y;
int cursor_hot_x, cursor_hot_y;
} TDSCContext;


enum TDSCCursorFormat {
CUR_FMT_MONO = 0x01010004,
CUR_FMT_BGRA = 0x20010004,
CUR_FMT_RGBA = 0x20010008,
};

static av_cold int tdsc_close(AVCodecContext *avctx)
{
TDSCContext *ctx = avctx->priv_data;

av_frame_free(&ctx->refframe);
av_frame_free(&ctx->jpgframe);
av_freep(&ctx->deflatebuffer);
av_freep(&ctx->tilebuffer);
av_freep(&ctx->cursor);
avcodec_free_context(&ctx->jpeg_avctx);

return 0;
}

static av_cold int tdsc_init(AVCodecContext *avctx)
{
TDSCContext *ctx = avctx->priv_data;
const AVCodec *codec;
int ret;

avctx->pix_fmt = AV_PIX_FMT_BGR24;


if (!(avctx->width && avctx->height)) {
av_log(avctx, AV_LOG_ERROR, "Video size not set.\n");
return AVERROR_INVALIDDATA;
}


ctx->deflatelen = avctx->width * avctx->height * (3 + 1);
ret = av_reallocp(&ctx->deflatebuffer, ctx->deflatelen);
if (ret < 0)
return ret;


ctx->refframe = av_frame_alloc();
ctx->jpgframe = av_frame_alloc();
if (!ctx->refframe || !ctx->jpgframe)
return AVERROR(ENOMEM);


codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
if (!codec)
return AVERROR_BUG;
ctx->jpeg_avctx = avcodec_alloc_context3(codec);
if (!ctx->jpeg_avctx)
return AVERROR(ENOMEM);
ctx->jpeg_avctx->flags = avctx->flags;
ctx->jpeg_avctx->flags2 = avctx->flags2;
ctx->jpeg_avctx->dct_algo = avctx->dct_algo;
ctx->jpeg_avctx->idct_algo = avctx->idct_algo;
ret = ff_codec_open2_recursive(ctx->jpeg_avctx, codec, NULL);
if (ret < 0)
return ret;


ctx->refframe->format = avctx->pix_fmt;

return 0;
}

#define APPLY_ALPHA(src, new, alpha) src = (src * (256 - alpha) + new * alpha) >> 8



static void tdsc_paint_cursor(AVCodecContext *avctx, uint8_t *dst, int stride)
{
TDSCContext *ctx = avctx->priv_data;
const uint8_t *cursor = ctx->cursor;
int x = ctx->cursor_x - ctx->cursor_hot_x;
int y = ctx->cursor_y - ctx->cursor_hot_y;
int w = ctx->cursor_w;
int h = ctx->cursor_h;
int i, j;

if (!ctx->cursor)
return;

if (x + w > ctx->width)
w = ctx->width - x;
if (y + h > ctx->height)
h = ctx->height - y;
if (x < 0) {
w += x;
cursor += -x * 4;
} else {
dst += x * 3;
}
if (y < 0) {
h += y;
cursor += -y * ctx->cursor_stride;
} else {
dst += y * stride;
}
if (w < 0 || h < 0)
return;

for (j = 0; j < h; j++) {
for (i = 0; i < w; i++) {
uint8_t alpha = cursor[i * 4];
APPLY_ALPHA(dst[i * 3 + 0], cursor[i * 4 + 1], alpha);
APPLY_ALPHA(dst[i * 3 + 1], cursor[i * 4 + 2], alpha);
APPLY_ALPHA(dst[i * 3 + 2], cursor[i * 4 + 3], alpha);
}
dst += stride;
cursor += ctx->cursor_stride;
}
}


static int tdsc_load_cursor(AVCodecContext *avctx)
{
TDSCContext *ctx = avctx->priv_data;
int i, j, k, ret, cursor_fmt;
uint8_t *dst;

ctx->cursor_hot_x = bytestream2_get_le16(&ctx->gbc);
ctx->cursor_hot_y = bytestream2_get_le16(&ctx->gbc);
ctx->cursor_w = bytestream2_get_le16(&ctx->gbc);
ctx->cursor_h = bytestream2_get_le16(&ctx->gbc);

ctx->cursor_stride = FFALIGN(ctx->cursor_w, 32) * 4;
cursor_fmt = bytestream2_get_le32(&ctx->gbc);

if (ctx->cursor_x >= avctx->width || ctx->cursor_y >= avctx->height) {
av_log(avctx, AV_LOG_ERROR,
"Invalid cursor position (%d.%d outside %dx%d).\n",
ctx->cursor_x, ctx->cursor_y, avctx->width, avctx->height);
return AVERROR_INVALIDDATA;
}
if (ctx->cursor_w < 1 || ctx->cursor_w > 256 ||
ctx->cursor_h < 1 || ctx->cursor_h > 256) {
av_log(avctx, AV_LOG_ERROR,
"Invalid cursor dimensions %dx%d.\n",
ctx->cursor_w, ctx->cursor_h);
return AVERROR_INVALIDDATA;
}
if (ctx->cursor_hot_x > ctx->cursor_w ||
ctx->cursor_hot_y > ctx->cursor_h) {
av_log(avctx, AV_LOG_WARNING, "Invalid hotspot position %d.%d.\n",
ctx->cursor_hot_x, ctx->cursor_hot_y);
ctx->cursor_hot_x = FFMIN(ctx->cursor_hot_x, ctx->cursor_w - 1);
ctx->cursor_hot_y = FFMIN(ctx->cursor_hot_y, ctx->cursor_h - 1);
}

ret = av_reallocp(&ctx->cursor, ctx->cursor_stride * ctx->cursor_h);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "Cannot allocate cursor buffer.\n");
return ret;
}

dst = ctx->cursor;

switch (cursor_fmt) {
case CUR_FMT_MONO:
for (j = 0; j < ctx->cursor_h; j++) {
for (i = 0; i < ctx->cursor_w; i += 32) {
uint32_t bits = bytestream2_get_be32(&ctx->gbc);
for (k = 0; k < 32; k++) {
dst[0] = !!(bits & 0x80000000);
dst += 4;
bits <<= 1;
}
}
dst += ctx->cursor_stride - ctx->cursor_w * 4;
}

dst = ctx->cursor;
for (j = 0; j < ctx->cursor_h; j++) {
for (i = 0; i < ctx->cursor_w; i += 32) {
uint32_t bits = bytestream2_get_be32(&ctx->gbc);
for (k = 0; k < 32; k++) {
int mask_bit = !!(bits & 0x80000000);
switch (dst[0] * 2 + mask_bit) {
case 0:
dst[0] = 0xFF;
dst[1] = 0x00;
dst[2] = 0x00;
dst[3] = 0x00;
break;
case 1:
dst[0] = 0xFF;
dst[1] = 0xFF;
dst[2] = 0xFF;
dst[3] = 0xFF;
break;
default:
dst[0] = 0x00;
dst[1] = 0x00;
dst[2] = 0x00;
dst[3] = 0x00;
}
dst += 4;
bits <<= 1;
}
}
dst += ctx->cursor_stride - ctx->cursor_w * 4;
}
break;
case CUR_FMT_BGRA:
case CUR_FMT_RGBA:

bytestream2_skip(&ctx->gbc,
ctx->cursor_h * (FFALIGN(ctx->cursor_w, 32) >> 3));
if (cursor_fmt & 8) { 
for (j = 0; j < ctx->cursor_h; j++) {
for (i = 0; i < ctx->cursor_w; i++) {
int val = bytestream2_get_be32(&ctx->gbc);
*dst++ = val >> 24;
*dst++ = val >> 16;
*dst++ = val >> 8;
*dst++ = val >> 0;
}
dst += ctx->cursor_stride - ctx->cursor_w * 4;
}
} else { 
for (j = 0; j < ctx->cursor_h; j++) {
for (i = 0; i < ctx->cursor_w; i++) {
int val = bytestream2_get_be32(&ctx->gbc);
*dst++ = val >> 0;
*dst++ = val >> 24;
*dst++ = val >> 16;
*dst++ = val >> 8;
}
dst += ctx->cursor_stride - ctx->cursor_w * 4;
}
}
break;
default:
avpriv_request_sample(avctx, "Cursor format %08x", cursor_fmt);
return AVERROR_PATCHWELCOME;
}

return 0;
}


static inline void tdsc_yuv2rgb(uint8_t *out, int Y, int U, int V)
{
out[0] = av_clip_uint8(Y + ( 91881 * V + 32768 >> 16));
out[1] = av_clip_uint8(Y + (-22554 * U - 46802 * V + 32768 >> 16));
out[2] = av_clip_uint8(Y + (116130 * U + 32768 >> 16));
}


static av_always_inline void tdsc_blit(uint8_t *dst, int dst_stride,
const uint8_t *srcy, int srcy_stride,
const uint8_t *srcu, const uint8_t *srcv,
int srcuv_stride, int width, int height)
{
int col, line;
for (line = 0; line < height; line++) {
for (col = 0; col < width; col++)
tdsc_yuv2rgb(dst + col * 3, srcy[col],
srcu[col >> 1] - 128, srcv[col >> 1] - 128);

dst += dst_stride;
srcy += srcy_stride;
srcu += srcuv_stride * (line & 1);
srcv += srcuv_stride * (line & 1);
}
}


static int tdsc_decode_jpeg_tile(AVCodecContext *avctx, int tile_size,
int x, int y, int w, int h)
{
TDSCContext *ctx = avctx->priv_data;
AVPacket jpkt;
int ret;


av_init_packet(&jpkt);
jpkt.data = ctx->tilebuffer;
jpkt.size = tile_size;

ret = avcodec_send_packet(ctx->jpeg_avctx, &jpkt);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "Error submitting a packet for decoding\n");
return ret;
}

ret = avcodec_receive_frame(ctx->jpeg_avctx, ctx->jpgframe);
if (ret < 0 || ctx->jpgframe->format != AV_PIX_FMT_YUVJ420P) {
av_log(avctx, AV_LOG_ERROR,
"JPEG decoding error (%d).\n", ret);


if (avctx->err_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
else
return 0;
}


tdsc_blit(ctx->refframe->data[0] + x * 3 + ctx->refframe->linesize[0] * y,
ctx->refframe->linesize[0],
ctx->jpgframe->data[0], ctx->jpgframe->linesize[0],
ctx->jpgframe->data[1], ctx->jpgframe->data[2],
ctx->jpgframe->linesize[1], w, h);

av_frame_unref(ctx->jpgframe);

return 0;
}


static int tdsc_decode_tiles(AVCodecContext *avctx, int number_tiles)
{
TDSCContext *ctx = avctx->priv_data;
int i;


for (i = 0; i < number_tiles; i++) {
int tile_size;
int tile_mode;
int x, y, w, h;
int ret;

if (bytestream2_get_bytes_left(&ctx->gbc) < 4 ||
bytestream2_get_le32(&ctx->gbc) != MKTAG('T','D','S','B') ||
bytestream2_get_bytes_left(&ctx->gbc) < TDSB_HEADER_SIZE - 4) {
av_log(avctx, AV_LOG_ERROR, "TDSB tag is too small.\n");
return AVERROR_INVALIDDATA;
}

tile_size = bytestream2_get_le32(&ctx->gbc);
if (bytestream2_get_bytes_left(&ctx->gbc) < tile_size)
return AVERROR_INVALIDDATA;

tile_mode = bytestream2_get_le32(&ctx->gbc);
bytestream2_skip(&ctx->gbc, 4); 
x = bytestream2_get_le32(&ctx->gbc);
y = bytestream2_get_le32(&ctx->gbc);
w = bytestream2_get_le32(&ctx->gbc) - x;
h = bytestream2_get_le32(&ctx->gbc) - y;

if (x >= ctx->width || y >= ctx->height) {
av_log(avctx, AV_LOG_ERROR,
"Invalid tile position (%d.%d outside %dx%d).\n",
x, y, ctx->width, ctx->height);
return AVERROR_INVALIDDATA;
}
if (x + w > ctx->width || y + h > ctx->height) {
av_log(avctx, AV_LOG_ERROR,
"Invalid tile size %dx%d\n", w, h);
return AVERROR_INVALIDDATA;
}

ret = av_reallocp(&ctx->tilebuffer, tile_size);
if (!ctx->tilebuffer)
return ret;

bytestream2_get_buffer(&ctx->gbc, ctx->tilebuffer, tile_size);

if (tile_mode == MKTAG('G','E','P','J')) {

ret = tdsc_decode_jpeg_tile(avctx, tile_size, x, y, w, h);
if (ret < 0)
return ret;
} else if (tile_mode == MKTAG(' ','W','A','R')) {

av_image_copy_plane(ctx->refframe->data[0] + x * 3 +
ctx->refframe->linesize[0] * y,
ctx->refframe->linesize[0], ctx->tilebuffer,
w * 3, w * 3, h);
} else {
av_log(avctx, AV_LOG_ERROR, "Unknown tile type %08x.\n", tile_mode);
return AVERROR_INVALIDDATA;
}
av_log(avctx, AV_LOG_DEBUG, "Tile %d, %dx%d (%d.%d)\n", i, w, h, x, y);
}

return 0;
}

static int tdsc_parse_tdsf(AVCodecContext *avctx, int number_tiles)
{
TDSCContext *ctx = avctx->priv_data;
int ret, w, h, init_refframe = !ctx->refframe->data[0];



if (bytestream2_get_le32(&ctx->gbc) != BITMAPINFOHEADER_SIZE)
return AVERROR_INVALIDDATA;


w = bytestream2_get_le32(&ctx->gbc);
h = -bytestream2_get_le32(&ctx->gbc);

if (bytestream2_get_le16(&ctx->gbc) != 1 || 
bytestream2_get_le16(&ctx->gbc) != 24) 
return AVERROR_INVALIDDATA;

bytestream2_skip(&ctx->gbc, 24); 


if (avctx->width != w || avctx->height != h) {
av_log(avctx, AV_LOG_DEBUG, "Size update %dx%d -> %d%d.\n",
avctx->width, avctx->height, ctx->width, ctx->height);
ret = ff_set_dimensions(avctx, w, h);
if (ret < 0)
return ret;
init_refframe = 1;
}
ctx->refframe->width = ctx->width = w;
ctx->refframe->height = ctx->height = h;


if (init_refframe) {
ret = av_frame_get_buffer(ctx->refframe, 32);
if (ret < 0)
return ret;
}


return tdsc_decode_tiles(avctx, number_tiles);
}

static int tdsc_parse_dtsm(AVCodecContext *avctx)
{
TDSCContext *ctx = avctx->priv_data;
int ret;
int action = bytestream2_get_le32(&ctx->gbc);

bytestream2_skip(&ctx->gbc, 4); 

if (action == 2 || action == 3) {

ctx->cursor_x = bytestream2_get_le32(&ctx->gbc);
ctx->cursor_y = bytestream2_get_le32(&ctx->gbc);


if (action == 3) {
ret = tdsc_load_cursor(avctx);

if (ret < 0 && (avctx->err_recognition & AV_EF_EXPLODE))
return ret;
}
} else {
avpriv_request_sample(avctx, "Cursor action %d", action);
}

return 0;
}

static int tdsc_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
TDSCContext *ctx = avctx->priv_data;
AVFrame *frame = data;
int ret, tag_header, keyframe = 0;
uLongf dlen;


if (ctx->width != avctx->width || ctx->height != avctx->height) {
ctx->deflatelen = avctx->width * avctx->height * (3 + 1);
ret = av_reallocp(&ctx->deflatebuffer, ctx->deflatelen);
if (ret < 0)
return ret;
}
dlen = ctx->deflatelen;


ret = uncompress(ctx->deflatebuffer, &dlen, avpkt->data, avpkt->size);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "Deflate error %d.\n", ret);
return AVERROR_UNKNOWN;
}

bytestream2_init(&ctx->gbc, ctx->deflatebuffer, dlen);


if (bytestream2_get_bytes_left(&ctx->gbc) < 4 + 4) {
av_log(avctx, AV_LOG_ERROR, "Frame is too small.\n");
return AVERROR_INVALIDDATA;
}


tag_header = bytestream2_get_le32(&ctx->gbc);

if (tag_header == MKTAG('T','D','S','F')) {
int number_tiles;
if (bytestream2_get_bytes_left(&ctx->gbc) < TDSF_HEADER_SIZE) {
av_log(avctx, AV_LOG_ERROR, "TDSF tag is too small.\n");
return AVERROR_INVALIDDATA;
}

number_tiles = bytestream2_get_le32(&ctx->gbc);

bytestream2_skip(&ctx->gbc, 4); 
keyframe = bytestream2_get_le32(&ctx->gbc) == 0x30;

ret = tdsc_parse_tdsf(avctx, number_tiles);
if (ret < 0)
return ret;


if (bytestream2_get_bytes_left(&ctx->gbc) >= 4 + 4)
tag_header = bytestream2_get_le32(&ctx->gbc);
}


if (tag_header == MKTAG('D','T','S','M')) {

int tag_size = bytestream2_get_le32(&ctx->gbc);

if (bytestream2_get_bytes_left(&ctx->gbc) < tag_size) {
av_log(avctx, AV_LOG_ERROR, "DTSM tag is too small.\n");
return AVERROR_INVALIDDATA;
}

ret = tdsc_parse_dtsm(avctx);
if (ret < 0)
return ret;
}


ret = ff_get_buffer(avctx, frame, 0);
if (ret < 0)
return ret;

ret = av_frame_copy(frame, ctx->refframe);
if (ret < 0)
return ret;


tdsc_paint_cursor(avctx, frame->data[0], frame->linesize[0]);


if (keyframe) {
frame->pict_type = AV_PICTURE_TYPE_I;
frame->key_frame = 1;
} else {
frame->pict_type = AV_PICTURE_TYPE_P;
}
*got_frame = 1;

return avpkt->size;
}

AVCodec ff_tdsc_decoder = {
.name = "tdsc",
.long_name = NULL_IF_CONFIG_SMALL("TDSC"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_TDSC,
.init = tdsc_init,
.decode = tdsc_decode_frame,
.close = tdsc_close,
.priv_data_size = sizeof(TDSCContext),
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
};
