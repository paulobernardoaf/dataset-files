



















#include "libavutil/common.h"
#include "get_bits.h"
#include "rtjpeg.h"

#define PUT_COEFF(c) i = scan[coeff--]; block[i] = (c) * quant[i];




#define ALIGN(a) n = (-get_bits_count(gb)) & (a - 1); if (n) {skip_bits(gb, n);}















static inline int get_block(GetBitContext *gb, int16_t *block, const uint8_t *scan,
const uint32_t *quant) {
int coeff, i, n;
int8_t ac;
uint8_t dc = get_bits(gb, 8);


if (dc == 255)
return 0;


coeff = get_bits(gb, 6);
if (get_bits_left(gb) < (coeff << 1))
return AVERROR_INVALIDDATA;



memset(block, 0, 64 * sizeof(int16_t));


while (coeff) {
ac = get_sbits(gb, 2);
if (ac == -2)
break; 
PUT_COEFF(ac);
}


ALIGN(4);
if (get_bits_left(gb) < (coeff << 2))
return AVERROR_INVALIDDATA;
while (coeff) {
ac = get_sbits(gb, 4);
if (ac == -8)
break; 
PUT_COEFF(ac);
}


ALIGN(8);
if (get_bits_left(gb) < (coeff << 3))
return AVERROR_INVALIDDATA;
while (coeff) {
ac = get_sbits(gb, 8);
PUT_COEFF(ac);
}

PUT_COEFF(dc);
return 1;
}










int ff_rtjpeg_decode_frame_yuv420(RTJpegContext *c, AVFrame *f,
const uint8_t *buf, int buf_size) {
GetBitContext gb;
int w = c->w / 16, h = c->h / 16;
int x, y, ret;
uint8_t *y1 = f->data[0], *y2 = f->data[0] + 8 * f->linesize[0];
uint8_t *u = f->data[1], *v = f->data[2];

if ((ret = init_get_bits8(&gb, buf, buf_size)) < 0)
return ret;

for (y = 0; y < h; y++) {
for (x = 0; x < w; x++) {
#define BLOCK(quant, dst, stride) do { int res = get_block(&gb, block, c->scan, quant); if (res < 0) return res; if (res > 0) c->idsp.idct_put(dst, stride, block); } while (0)






int16_t *block = c->block;
BLOCK(c->lquant, y1, f->linesize[0]);
y1 += 8;
BLOCK(c->lquant, y1, f->linesize[0]);
y1 += 8;
BLOCK(c->lquant, y2, f->linesize[0]);
y2 += 8;
BLOCK(c->lquant, y2, f->linesize[0]);
y2 += 8;
BLOCK(c->cquant, u, f->linesize[1]);
u += 8;
BLOCK(c->cquant, v, f->linesize[2]);
v += 8;
}
y1 += 2 * 8 * (f->linesize[0] - w);
y2 += 2 * 8 * (f->linesize[0] - w);
u += 8 * (f->linesize[1] - w);
v += 8 * (f->linesize[2] - w);
}
return get_bits_count(&gb) / 8;
}











void ff_rtjpeg_decode_init(RTJpegContext *c, int width, int height,
const uint32_t *lquant, const uint32_t *cquant) {
int i;
for (i = 0; i < 64; i++) {
int p = c->idsp.idct_permutation[i];
c->lquant[p] = lquant[i];
c->cquant[p] = cquant[i];
}
c->w = width;
c->h = height;
}

void ff_rtjpeg_init(RTJpegContext *c, AVCodecContext *avctx)
{
int i;

ff_idctdsp_init(&c->idsp, avctx);

for (i = 0; i < 64; i++) {
int z = ff_zigzag_direct[i];
z = ((z << 3) | (z >> 3)) & 63; 


c->scan[i] = c->idsp.idct_permutation[z];
}
}
