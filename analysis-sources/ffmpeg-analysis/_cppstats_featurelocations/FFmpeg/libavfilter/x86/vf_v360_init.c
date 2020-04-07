

















#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/v360.h"

void ff_remap1_8bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

void ff_remap2_8bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

void ff_remap3_8bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

void ff_remap4_8bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

void ff_remap1_16bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

void ff_remap2_16bit_line_avx2(uint8_t *dst, int width, const uint8_t *src, ptrdiff_t in_linesize,
const int16_t *const u, const int16_t *const v, const int16_t *const ker);

av_cold void ff_v360_init_x86(V360Context *s, int depth)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_AVX2_FAST(cpu_flags) && s->interp == NEAREST && depth <= 8)
s->remap_line = ff_remap1_8bit_line_avx2;

if (EXTERNAL_AVX2_FAST(cpu_flags) && s->interp == BILINEAR && depth <= 8)
s->remap_line = ff_remap2_8bit_line_avx2;

if (EXTERNAL_AVX2_FAST(cpu_flags) && s->interp == NEAREST && depth > 8)
s->remap_line = ff_remap1_16bit_line_avx2;

if (EXTERNAL_AVX2_FAST(cpu_flags) && s->interp == BILINEAR && depth > 8)
s->remap_line = ff_remap2_16bit_line_avx2;

#if ARCH_X86_64
if (EXTERNAL_AVX2_FAST(cpu_flags) && s->interp == LAGRANGE9 && depth <= 8)
s->remap_line = ff_remap3_8bit_line_avx2;

if (EXTERNAL_AVX2_FAST(cpu_flags) && (s->interp == BICUBIC ||
s->interp == LANCZOS ||
s->interp == SPLINE16 ||
s->interp == GAUSSIAN) && depth <= 8)
s->remap_line = ff_remap4_8bit_line_avx2;
#endif
}
