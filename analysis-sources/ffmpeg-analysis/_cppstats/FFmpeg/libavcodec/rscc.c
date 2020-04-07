#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#define TILE_SIZE 8
typedef struct Tile {
int x, y;
int w, h;
} Tile;
typedef struct RsccContext {
GetByteContext gbc;
AVFrame *reference;
Tile *tiles;
unsigned int tiles_size;
int component_size;
uint8_t palette[AVPALETTE_SIZE];
uint8_t *inflated_buf;
uLongf inflated_size;
int valid_pixels;
} RsccContext;
static av_cold int rscc_init(AVCodecContext *avctx)
{
RsccContext *ctx = avctx->priv_data;
int ret = av_image_check_size(avctx->width, avctx->height, 0, avctx);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n",
avctx->width, avctx->height);
return ret;
}
ctx->reference = av_frame_alloc();
if (!ctx->reference)
return AVERROR(ENOMEM);
if (avctx->codec_tag == MKTAG('I', 'S', 'C', 'C')) {
if (avctx->extradata && avctx->extradata_size == 4) {
if ((avctx->extradata[0] >> 1) & 1) {
avctx->pix_fmt = AV_PIX_FMT_BGRA;
ctx->component_size = 4;
} else {
avctx->pix_fmt = AV_PIX_FMT_BGR24;
ctx->component_size = 3;
}
} else {
avctx->pix_fmt = AV_PIX_FMT_BGRA;
ctx->component_size = 4;
}
} else if (avctx->codec_tag == MKTAG('R', 'S', 'C', 'C')) {
ctx->component_size = avctx->bits_per_coded_sample / 8;
switch (avctx->bits_per_coded_sample) {
case 8:
avctx->pix_fmt = AV_PIX_FMT_PAL8;
break;
case 16:
avctx->pix_fmt = AV_PIX_FMT_RGB555LE;
break;
case 24:
avctx->pix_fmt = AV_PIX_FMT_BGR24;
break;
case 32:
avctx->pix_fmt = AV_PIX_FMT_BGR0;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Invalid bits per pixel value (%d)\n",
avctx->bits_per_coded_sample);
return AVERROR_INVALIDDATA;
}
} else {
avctx->pix_fmt = AV_PIX_FMT_BGR0;
ctx->component_size = 4;
av_log(avctx, AV_LOG_WARNING, "Invalid codec tag\n");
}
ctx->inflated_size = avctx->width * avctx->height * ctx->component_size;
ctx->inflated_buf = av_malloc(ctx->inflated_size);
if (!ctx->inflated_buf)
return AVERROR(ENOMEM);
return 0;
}
static av_cold int rscc_close(AVCodecContext *avctx)
{
RsccContext *ctx = avctx->priv_data;
av_freep(&ctx->tiles);
av_freep(&ctx->inflated_buf);
av_frame_free(&ctx->reference);
return 0;
}
static int rscc_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
RsccContext *ctx = avctx->priv_data;
GetByteContext *gbc = &ctx->gbc;
GetByteContext tiles_gbc;
AVFrame *frame = data;
const uint8_t *pixels, *raw;
uint8_t *inflated_tiles = NULL;
int tiles_nb, packed_size, pixel_size = 0;
int i, ret = 0;
bytestream2_init(gbc, avpkt->data, avpkt->size);
if (bytestream2_get_bytes_left(gbc) < 12) {
av_log(avctx, AV_LOG_ERROR, "Packet too small (%d)\n", avpkt->size);
return AVERROR_INVALIDDATA;
}
tiles_nb = bytestream2_get_le16(gbc);
if (tiles_nb == 0) {
av_log(avctx, AV_LOG_DEBUG, "no tiles\n");
return avpkt->size;
}
av_fast_malloc(&ctx->tiles, &ctx->tiles_size,
tiles_nb * sizeof(*ctx->tiles));
if (!ctx->tiles) {
ret = AVERROR(ENOMEM);
goto end;
}
av_log(avctx, AV_LOG_DEBUG, "Frame with %d tiles.\n", tiles_nb);
if (tiles_nb > 5) {
uLongf packed_tiles_size;
if (tiles_nb < 32)
packed_tiles_size = bytestream2_get_byte(gbc);
else
packed_tiles_size = bytestream2_get_le16(gbc);
ff_dlog(avctx, "packed tiles of size %lu.\n", packed_tiles_size);
if (packed_tiles_size != tiles_nb * TILE_SIZE) {
uLongf length = tiles_nb * TILE_SIZE;
if (bytestream2_get_bytes_left(gbc) < packed_tiles_size) {
ret = AVERROR_INVALIDDATA;
goto end;
}
inflated_tiles = av_malloc(length);
if (!inflated_tiles) {
ret = AVERROR(ENOMEM);
goto end;
}
ret = uncompress(inflated_tiles, &length,
gbc->buffer, packed_tiles_size);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "Tile deflate error %d.\n", ret);
ret = AVERROR_UNKNOWN;
goto end;
}
bytestream2_skip(gbc, packed_tiles_size);
bytestream2_init(&tiles_gbc, inflated_tiles, length);
gbc = &tiles_gbc;
}
}
for (i = 0; i < tiles_nb; i++) {
ctx->tiles[i].x = bytestream2_get_le16(gbc);
ctx->tiles[i].w = bytestream2_get_le16(gbc);
ctx->tiles[i].y = bytestream2_get_le16(gbc);
ctx->tiles[i].h = bytestream2_get_le16(gbc);
if (pixel_size + ctx->tiles[i].w * (int64_t)ctx->tiles[i].h * ctx->component_size > INT_MAX) {
av_log(avctx, AV_LOG_ERROR, "Invalid tile dimensions\n");
ret = AVERROR_INVALIDDATA;
goto end;
}
pixel_size += ctx->tiles[i].w * ctx->tiles[i].h * ctx->component_size;
ff_dlog(avctx, "tile %d orig(%d,%d) %dx%d.\n", i,
ctx->tiles[i].x, ctx->tiles[i].y,
ctx->tiles[i].w, ctx->tiles[i].h);
if (ctx->tiles[i].w == 0 || ctx->tiles[i].h == 0) {
av_log(avctx, AV_LOG_ERROR,
"invalid tile %d at (%d.%d) with size %dx%d.\n", i,
ctx->tiles[i].x, ctx->tiles[i].y,
ctx->tiles[i].w, ctx->tiles[i].h);
ret = AVERROR_INVALIDDATA;
goto end;
} else if (ctx->tiles[i].x + ctx->tiles[i].w > avctx->width ||
ctx->tiles[i].y + ctx->tiles[i].h > avctx->height) {
av_log(avctx, AV_LOG_ERROR,
"out of bounds tile %d at (%d.%d) with size %dx%d.\n", i,
ctx->tiles[i].x, ctx->tiles[i].y,
ctx->tiles[i].w, ctx->tiles[i].h);
ret = AVERROR_INVALIDDATA;
goto end;
}
}
gbc = &ctx->gbc;
if (pixel_size < 0x100)
packed_size = bytestream2_get_byte(gbc);
else if (pixel_size < 0x10000)
packed_size = bytestream2_get_le16(gbc);
else if (pixel_size < 0x1000000)
packed_size = bytestream2_get_le24(gbc);
else
packed_size = bytestream2_get_le32(gbc);
ff_dlog(avctx, "pixel_size %d packed_size %d.\n", pixel_size, packed_size);
if (packed_size < 0) {
av_log(avctx, AV_LOG_ERROR, "Invalid tile size %d\n", packed_size);
ret = AVERROR_INVALIDDATA;
goto end;
}
if (pixel_size == packed_size) {
if (bytestream2_get_bytes_left(gbc) < pixel_size) {
av_log(avctx, AV_LOG_ERROR, "Insufficient input for %d\n", pixel_size);
ret = AVERROR_INVALIDDATA;
goto end;
}
pixels = gbc->buffer;
} else {
uLongf len = ctx->inflated_size;
if (bytestream2_get_bytes_left(gbc) < packed_size) {
av_log(avctx, AV_LOG_ERROR, "Insufficient input for %d\n", packed_size);
ret = AVERROR_INVALIDDATA;
goto end;
}
ret = uncompress(ctx->inflated_buf, &len, gbc->buffer, packed_size);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "Pixel deflate error %d.\n", ret);
ret = AVERROR_UNKNOWN;
goto end;
}
pixels = ctx->inflated_buf;
}
ret = ff_reget_buffer(avctx, ctx->reference, 0);
if (ret < 0)
goto end;
raw = pixels;
for (i = 0; i < tiles_nb; i++) {
uint8_t *dst = ctx->reference->data[0] + ctx->reference->linesize[0] *
(avctx->height - ctx->tiles[i].y - 1) +
ctx->tiles[i].x * ctx->component_size;
av_image_copy_plane(dst, -1 * ctx->reference->linesize[0],
raw, ctx->tiles[i].w * ctx->component_size,
ctx->tiles[i].w * ctx->component_size,
ctx->tiles[i].h);
raw += ctx->tiles[i].w * ctx->component_size * ctx->tiles[i].h;
}
ret = av_frame_ref(frame, ctx->reference);
if (ret < 0)
goto end;
if (pixel_size == ctx->inflated_size) {
frame->pict_type = AV_PICTURE_TYPE_I;
frame->key_frame = 1;
} else {
frame->pict_type = AV_PICTURE_TYPE_P;
}
if (avctx->pix_fmt == AV_PIX_FMT_PAL8) {
int size;
const uint8_t *palette = av_packet_get_side_data(avpkt,
AV_PKT_DATA_PALETTE,
&size);
if (palette && size == AVPALETTE_SIZE) {
frame->palette_has_changed = 1;
memcpy(ctx->palette, palette, AVPALETTE_SIZE);
} else if (palette) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
}
memcpy (frame->data[1], ctx->palette, AVPALETTE_SIZE);
}
if (ctx->valid_pixels < ctx->inflated_size)
ctx->valid_pixels += pixel_size;
if (ctx->valid_pixels >= ctx->inflated_size * (100 - avctx->discard_damaged_percentage) / 100)
*got_frame = 1;
ret = avpkt->size;
end:
av_free(inflated_tiles);
return ret;
}
AVCodec ff_rscc_decoder = {
.name = "rscc",
.long_name = NULL_IF_CONFIG_SMALL("innoHeim/Rsupport Screen Capture Codec"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_RSCC,
.init = rscc_init,
.decode = rscc_decode_frame,
.close = rscc_close,
.priv_data_size = sizeof(RsccContext),
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
};
