#include "bytestream.h"
#include "internal.h"
#define CLIP(v, depth) av_clip(v, 1<<(depth-8), ((1<<depth)-(1<<(depth-8))-1))
#define WRITE_PIXELS(a, b, c, depth) do { val = CLIP(*a++, depth) << (10-depth); val |= (CLIP(*b++, depth) << (20-depth)) | (CLIP(*c++, depth) << (30-depth)); AV_WL32(dst, val); dst += 4; } while (0)
static void RENAME(v210_enc)(AVCodecContext *avctx,
uint8_t *dst, const AVFrame *pic)
{
V210EncContext *s = avctx->priv_data;
int aligned_width = ((avctx->width + 47) / 48) * 48;
int stride = aligned_width * 8 / 3;
int line_padding = stride - ((avctx->width * 8 + 11) / 12) * 4;
int h, w;
const TYPE *y = (const TYPE *)pic->data[0];
const TYPE *u = (const TYPE *)pic->data[1];
const TYPE *v = (const TYPE *)pic->data[2];
const int sample_size = 6 * s->RENAME(sample_factor);
const int sample_w = avctx->width / sample_size;
for (h = 0; h < avctx->height; h++) {
uint32_t val;
w = sample_w * sample_size;
s->RENAME(pack_line)(y, u, v, dst, w);
y += w;
u += w >> 1;
v += w >> 1;
dst += sample_w * 16 * s->RENAME(sample_factor);
for (; w < avctx->width - 5; w += 6) {
WRITE_PIXELS(u, y, v, DEPTH);
WRITE_PIXELS(y, u, y, DEPTH);
WRITE_PIXELS(v, y, u, DEPTH);
WRITE_PIXELS(y, v, y, DEPTH);
}
if (w < avctx->width - 1) {
WRITE_PIXELS(u, y, v, DEPTH);
val = CLIP(*y++, DEPTH) << (10-DEPTH);
if (w == avctx->width - 2) {
AV_WL32(dst, val);
dst += 4;
}
}
if (w < avctx->width - 3) {
val |= (CLIP(*u++, DEPTH) << (20-DEPTH)) | (CLIP(*y++, DEPTH) << (30-DEPTH));
AV_WL32(dst, val);
dst += 4;
val = CLIP(*v++, DEPTH) << (10-DEPTH) | (CLIP(*y++, DEPTH) << (20-DEPTH));
AV_WL32(dst, val);
dst += 4;
}
memset(dst, 0, line_padding);
dst += line_padding;
y += pic->linesize[0] / BYTES_PER_PIXEL - avctx->width;
u += pic->linesize[1] / BYTES_PER_PIXEL - avctx->width / 2;
v += pic->linesize[2] / BYTES_PER_PIXEL - avctx->width / 2;
}
}
