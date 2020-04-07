



















#include "libavutil/attributes.h"
#include "libavcodec/pixblockdsp.h"
#include "asm.h"

static void get_pixels_mvi(int16_t *restrict block,
const uint8_t *restrict pixels, ptrdiff_t stride)
{
int h = 8;

do {
uint64_t p;

p = ldq(pixels);
stq(unpkbw(p), block);
stq(unpkbw(p >> 32), block + 4);

pixels += stride;
block += 8;
} while (--h);
}

static void diff_pixels_mvi(int16_t *block, const uint8_t *s1, const uint8_t *s2,
ptrdiff_t stride)
{
int h = 8;
uint64_t mask = 0x4040;

mask |= mask << 16;
mask |= mask << 32;
do {
uint64_t x, y, c, d, a;
uint64_t signs;

x = ldq(s1);
y = ldq(s2);
c = cmpbge(x, y);
d = x - y;
a = zap(mask, c); 
d += 4 * a; 
signs = zap(-1, c);

stq(unpkbw(d) | (unpkbw(signs) << 8), block);
stq(unpkbw(d >> 32) | (unpkbw(signs >> 32) << 8), block + 4);

s1 += stride;
s2 += stride;
block += 8;
} while (--h);
}

av_cold void ff_pixblockdsp_init_alpha(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if (amask(AMASK_MVI) == 0) {
if (!high_bit_depth)
c->get_pixels = get_pixels_mvi;
c->diff_pixels = diff_pixels_mvi;
}
}
