

















#if !defined(AVCODEC_X86_HPELDSP_H)
#define AVCODEC_X86_HPELDSP_H

#include <stddef.h>
#include <stdint.h>

#include "libavcodec/hpeldsp.h"

void ff_avg_pixels8_x2_mmx(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_avg_pixels8_xy2_mmx(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_xy2_mmxext(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels8_xy2_ssse3(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_avg_pixels16_xy2_mmx(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_xy2_sse2(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_avg_pixels16_xy2_ssse3(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_put_pixels8_xy2_mmx(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels8_xy2_ssse3(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_xy2_mmx(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_xy2_sse2(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_xy2_ssse3(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

void ff_hpeldsp_vp3_init_x86(HpelDSPContext *c, int cpu_flags, int flags);

#endif 
