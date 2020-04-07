



















#include <stdint.h>

#include "libavutil/attributes.h"
#include "exrdsp.h"
#include "config.h"

static void reorder_pixels_scalar(uint8_t *dst, const uint8_t *src, ptrdiff_t size)
{
const uint8_t *t1 = src;
int half_size = size / 2;
const uint8_t *t2 = src + half_size;
uint8_t *s = dst;
int i;

for (i = 0; i < half_size; i++) {
*(s++) = *(t1++);
*(s++) = *(t2++);
}
}

static void predictor_scalar(uint8_t *src, ptrdiff_t size)
{
ptrdiff_t i;

for (i = 1; i < size; i++)
src[i] += src[i-1] - 128;
}

av_cold void ff_exrdsp_init(ExrDSPContext *c)
{
c->reorder_pixels = reorder_pixels_scalar;
c->predictor = predictor_scalar;

if (ARCH_X86)
ff_exrdsp_init_x86(c);
}
