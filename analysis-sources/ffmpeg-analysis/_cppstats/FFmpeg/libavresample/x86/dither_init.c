#include "config.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavresample/dither.h"
void ff_quantize_sse2(int16_t *dst, const float *src, float *dither, int len);
void ff_dither_int_to_float_rectangular_sse2(float *dst, int *src, int len);
void ff_dither_int_to_float_rectangular_avx(float *dst, int *src, int len);
void ff_dither_int_to_float_triangular_sse2(float *dst, int *src0, int len);
void ff_dither_int_to_float_triangular_avx(float *dst, int *src0, int len);
av_cold void ff_dither_init_x86(DitherDSPContext *ddsp,
enum AVResampleDitherMethod method)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags)) {
ddsp->quantize = ff_quantize_sse2;
ddsp->ptr_align = 16;
ddsp->samples_align = 8;
}
if (method == AV_RESAMPLE_DITHER_RECTANGULAR) {
if (EXTERNAL_SSE2(cpu_flags)) {
ddsp->dither_int_to_float = ff_dither_int_to_float_rectangular_sse2;
}
if (EXTERNAL_AVX_FAST(cpu_flags)) {
ddsp->dither_int_to_float = ff_dither_int_to_float_rectangular_avx;
}
} else {
if (EXTERNAL_SSE2(cpu_flags)) {
ddsp->dither_int_to_float = ff_dither_int_to_float_triangular_sse2;
}
if (EXTERNAL_AVX_FAST(cpu_flags)) {
ddsp->dither_int_to_float = ff_dither_int_to_float_triangular_avx;
}
}
}
