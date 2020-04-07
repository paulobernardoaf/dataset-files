#include "avcodec.h"
#include "get_bits.h"
#include "huffman.h"
#include "bytestream.h"
#include "bswapdsp.h"
#include "internal.h"
#include "thread.h"
#define FPS_TAG MKTAG('F', 'P', 'S', 'x')
#define VLC_BITS 11
typedef struct FrapsContext {
AVCodecContext *avctx;
BswapDSPContext bdsp;
uint8_t *tmpbuf;
int tmpbuf_size;
} FrapsContext;
static av_cold int decode_init(AVCodecContext *avctx)
{
FrapsContext * const s = avctx->priv_data;
s->avctx = avctx;
s->tmpbuf = NULL;
ff_bswapdsp_init(&s->bdsp);
return 0;
}
static int huff_cmp(const void *va, const void *vb)
{
const Node *a = va, *b = vb;
return (a->count - b->count)*256 + a->sym - b->sym;
}
static int fraps2_decode_plane(FrapsContext *s, uint8_t *dst, int stride, int w,
int h, const uint8_t *src, int size, int Uoff,
const int step)
{
int i, j, ret;
GetBitContext gb;
VLC vlc;
Node nodes[512];
for (i = 0; i < 256; i++)
nodes[i].count = bytestream_get_le32(&src);
size -= 1024;
if ((ret = ff_huff_build_tree(s->avctx, &vlc, 256, VLC_BITS,
nodes, huff_cmp,
FF_HUFFMAN_FLAG_ZERO_COUNT)) < 0)
return ret;
s->bdsp.bswap_buf((uint32_t *) s->tmpbuf,
(const uint32_t *) src, size >> 2);
if ((ret = init_get_bits8(&gb, s->tmpbuf, size)) < 0)
return ret;
for (j = 0; j < h; j++) {
for (i = 0; i < w*step; i += step) {
dst[i] = get_vlc2(&gb, vlc.table, VLC_BITS, 3);
if (j)
dst[i] += dst[i - stride];
else if (Uoff)
dst[i] += 0x80;
if (get_bits_left(&gb) < 0) {
ff_free_vlc(&vlc);
return AVERROR_INVALIDDATA;
}
}
dst += stride;
}
ff_free_vlc(&vlc);
return 0;
}
static int decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
FrapsContext * const s = avctx->priv_data;
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
ThreadFrame frame = { .f = data };
AVFrame * const f = data;
uint32_t header;
unsigned int version,header_size;
unsigned int x, y;
const uint32_t *buf32;
uint32_t *luma1,*luma2,*cb,*cr;
uint32_t offs[4];
int i, j, ret, is_chroma;
const int planes = 3;
int is_pal;
uint8_t *out;
if (buf_size < 4) {
av_log(avctx, AV_LOG_ERROR, "Packet is too short\n");
return AVERROR_INVALIDDATA;
}
header = AV_RL32(buf);
version = header & 0xff;
is_pal = buf[1] == 2 && version == 1;
header_size = (header & (1<<30))? 8 : 4; 
if (version > 5) {
avpriv_report_missing_feature(avctx, "Fraps version %u", version);
return AVERROR_PATCHWELCOME;
}
buf += header_size;
if (is_pal) {
unsigned needed_size = avctx->width * avctx->height + 1024;
needed_size += header_size;
if (buf_size != needed_size) {
av_log(avctx, AV_LOG_ERROR,
"Invalid frame length %d (should be %d)\n",
buf_size, needed_size);
return AVERROR_INVALIDDATA;
}
} else if (version < 2) {
unsigned needed_size = avctx->width * avctx->height * 3;
if (version == 0) needed_size /= 2;
needed_size += header_size;
if (header & (1U<<31)) {
*got_frame = 0;
return buf_size;
}
if (buf_size != needed_size) {
av_log(avctx, AV_LOG_ERROR,
"Invalid frame length %d (should be %d)\n",
buf_size, needed_size);
return AVERROR_INVALIDDATA;
}
} else {
if (buf_size == 8) {
*got_frame = 0;
return buf_size;
}
if (AV_RL32(buf) != FPS_TAG || buf_size < planes*1024 + 24) {
av_log(avctx, AV_LOG_ERROR, "error in data stream\n");
return AVERROR_INVALIDDATA;
}
for (i = 0; i < planes; i++) {
offs[i] = AV_RL32(buf + 4 + i * 4);
if (offs[i] >= buf_size - header_size || (i && offs[i] <= offs[i - 1] + 1024)) {
av_log(avctx, AV_LOG_ERROR, "plane %i offset is out of bounds\n", i);
return AVERROR_INVALIDDATA;
}
}
offs[planes] = buf_size - header_size;
for (i = 0; i < planes; i++) {
av_fast_padded_malloc(&s->tmpbuf, &s->tmpbuf_size, offs[i + 1] - offs[i] - 1024);
if (!s->tmpbuf)
return AVERROR(ENOMEM);
}
}
f->pict_type = AV_PICTURE_TYPE_I;
f->key_frame = 1;
avctx->pix_fmt = version & 1 ? is_pal ? AV_PIX_FMT_PAL8 : AV_PIX_FMT_BGR24 : AV_PIX_FMT_YUVJ420P;
avctx->color_range = version & 1 ? AVCOL_RANGE_UNSPECIFIED
: AVCOL_RANGE_JPEG;
avctx->colorspace = version & 1 ? AVCOL_SPC_UNSPECIFIED : AVCOL_SPC_BT709;
if ((ret = ff_thread_get_buffer(avctx, &frame, 0)) < 0)
return ret;
switch (version) {
case 0:
default:
if (((avctx->width % 8) != 0) || ((avctx->height % 2) != 0)) {
av_log(avctx, AV_LOG_ERROR, "Invalid frame size %dx%d\n",
avctx->width, avctx->height);
return AVERROR_INVALIDDATA;
}
buf32 = (const uint32_t*)buf;
for (y = 0; y < avctx->height / 2; y++) {
luma1 = (uint32_t*)&f->data[0][ y * 2 * f->linesize[0] ];
luma2 = (uint32_t*)&f->data[0][ (y * 2 + 1) * f->linesize[0] ];
cr = (uint32_t*)&f->data[1][ y * f->linesize[1] ];
cb = (uint32_t*)&f->data[2][ y * f->linesize[2] ];
for (x = 0; x < avctx->width; x += 8) {
*luma1++ = *buf32++;
*luma1++ = *buf32++;
*luma2++ = *buf32++;
*luma2++ = *buf32++;
*cr++ = *buf32++;
*cb++ = *buf32++;
}
}
break;
case 1:
if (is_pal) {
uint32_t *pal = (uint32_t *)f->data[1];
for (y = 0; y < 256; y++) {
pal[y] = AV_RL32(buf) | 0xFF000000;
buf += 4;
}
for (y = 0; y <avctx->height; y++)
memcpy(&f->data[0][y * f->linesize[0]],
&buf[y * avctx->width],
avctx->width);
} else {
for (y = 0; y<avctx->height; y++)
memcpy(&f->data[0][(avctx->height - y - 1) * f->linesize[0]],
&buf[y * avctx->width * 3],
3 * avctx->width);
}
break;
case 2:
case 4:
for (i = 0; i < planes; i++) {
is_chroma = !!i;
if ((ret = fraps2_decode_plane(s, f->data[i], f->linesize[i],
avctx->width >> is_chroma,
avctx->height >> is_chroma,
buf + offs[i], offs[i + 1] - offs[i],
is_chroma, 1)) < 0) {
av_log(avctx, AV_LOG_ERROR, "Error decoding plane %i\n", i);
return ret;
}
}
break;
case 3:
case 5:
for (i = 0; i < planes; i++) {
if ((ret = fraps2_decode_plane(s, f->data[0] + i + (f->linesize[0] * (avctx->height - 1)),
-f->linesize[0], avctx->width, avctx->height,
buf + offs[i], offs[i + 1] - offs[i], 0, 3)) < 0) {
av_log(avctx, AV_LOG_ERROR, "Error decoding plane %i\n", i);
return ret;
}
}
out = f->data[0];
for (j = 0; j < avctx->height; j++) {
uint8_t *line_end = out + 3*avctx->width;
while (out < line_end) {
out[0] += out[1];
out[2] += out[1];
out += 3;
}
out += f->linesize[0] - 3*avctx->width;
}
break;
}
*got_frame = 1;
return buf_size;
}
static av_cold int decode_end(AVCodecContext *avctx)
{
FrapsContext *s = (FrapsContext*)avctx->priv_data;
av_freep(&s->tmpbuf);
return 0;
}
AVCodec ff_fraps_decoder = {
.name = "fraps",
.long_name = NULL_IF_CONFIG_SMALL("Fraps"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_FRAPS,
.priv_data_size = sizeof(FrapsContext),
.init = decode_init,
.close = decode_end,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_FRAME_THREADS,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
