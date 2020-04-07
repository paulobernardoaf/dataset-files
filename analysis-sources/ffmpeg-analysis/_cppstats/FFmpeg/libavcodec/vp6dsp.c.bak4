
























#include "libavutil/common.h"
#include "vp56dsp.h"


void ff_vp6_filter_diag4_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
const int16_t *h_weights, const int16_t *v_weights)
{
int x, y;
int tmp[8*11];
int *t = tmp;

src -= stride;

for (y=0; y<11; y++) {
for (x=0; x<8; x++) {
t[x] = av_clip_uint8(( src[x-1] * h_weights[0]
+ src[x ] * h_weights[1]
+ src[x+1] * h_weights[2]
+ src[x+2] * h_weights[3] + 64) >> 7);
}
src += stride;
t += 8;
}

t = tmp + 8;
for (y=0; y<8; y++) {
for (x=0; x<8; x++) {
dst[x] = av_clip_uint8(( t[x-8 ] * v_weights[0]
+ t[x ] * v_weights[1]
+ t[x+8 ] * v_weights[2]
+ t[x+16] * v_weights[3] + 64) >> 7);
}
dst += stride;
t += 8;
}
}
