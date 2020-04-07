



















#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/exrdsp.h"

void ff_reorder_pixels_sse2(uint8_t *dst, const uint8_t *src, ptrdiff_t size);

void ff_reorder_pixels_avx2(uint8_t *dst, const uint8_t *src, ptrdiff_t size);

void ff_predictor_ssse3(uint8_t *src, ptrdiff_t size);

void ff_predictor_avx(uint8_t *src, ptrdiff_t size);

void ff_predictor_avx2(uint8_t *src, ptrdiff_t size);

av_cold void ff_exrdsp_init_x86(ExrDSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags)) {
dsp->reorder_pixels = ff_reorder_pixels_sse2;
}
if (EXTERNAL_SSSE3(cpu_flags)) {
dsp->predictor = ff_predictor_ssse3;
}
if (EXTERNAL_AVX(cpu_flags)) {
dsp->predictor = ff_predictor_avx;
}
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
dsp->reorder_pixels = ff_reorder_pixels_avx2;
dsp->predictor = ff_predictor_avx2;
}
}
