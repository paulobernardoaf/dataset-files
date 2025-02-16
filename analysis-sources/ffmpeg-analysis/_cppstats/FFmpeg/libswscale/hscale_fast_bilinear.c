#include "swscale_internal.h"
void ff_hyscale_fast_c(SwsContext *c, int16_t *dst, int dstWidth,
const uint8_t *src, int srcW, int xInc)
{
int i;
unsigned int xpos = 0;
for (i = 0; i < dstWidth; i++) {
register unsigned int xx = xpos >> 16;
register unsigned int xalpha = (xpos & 0xFFFF) >> 9;
dst[i] = (src[xx] << 7) + (src[xx + 1] - src[xx]) * xalpha;
xpos += xInc;
}
for (i=dstWidth-1; (i*xInc)>>16 >=srcW-1; i--)
dst[i] = src[srcW-1]*128;
}
void ff_hcscale_fast_c(SwsContext *c, int16_t *dst1, int16_t *dst2,
int dstWidth, const uint8_t *src1,
const uint8_t *src2, int srcW, int xInc)
{
int i;
unsigned int xpos = 0;
for (i = 0; i < dstWidth; i++) {
register unsigned int xx = xpos >> 16;
register unsigned int xalpha = (xpos & 0xFFFF) >> 9;
dst1[i] = (src1[xx] * (xalpha ^ 127) + src1[xx + 1] * xalpha);
dst2[i] = (src2[xx] * (xalpha ^ 127) + src2[xx + 1] * xalpha);
xpos += xInc;
}
for (i=dstWidth-1; (i*xInc)>>16 >=srcW-1; i--) {
dst1[i] = src1[srcW-1]*128;
dst2[i] = src2[srcW-1]*128;
}
}
