



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/avf_showcqt.h"

#define DECLARE_CQT_CALC(type) void ff_showcqt_cqt_calc_##type(FFTComplex *dst, const FFTComplex *src, const Coeffs *coeffs, int len, int fft_len)



DECLARE_CQT_CALC(sse);
DECLARE_CQT_CALC(sse3);
DECLARE_CQT_CALC(avx);
DECLARE_CQT_CALC(fma3);
DECLARE_CQT_CALC(fma4);

#define permute_coeffs_0 NULL

static void permute_coeffs_01452367(float *v, int len)
{
int k;
for (k = 0; k < len; k += 8) {
FFSWAP(float, v[k+2], v[k+4]);
FFSWAP(float, v[k+3], v[k+5]);
}
}

av_cold void ff_showcqt_init_x86(ShowCQTContext *s)
{
int cpuflags = av_get_cpu_flags();

#define SELECT_CQT_CALC(type, TYPE, align, perm) if (EXTERNAL_##TYPE(cpuflags)) { s->cqt_calc = ff_showcqt_cqt_calc_##type; s->cqt_align = align; s->permute_coeffs = permute_coeffs_##perm; }






SELECT_CQT_CALC(sse, SSE, 4, 0);
SELECT_CQT_CALC(sse3, SSE3_FAST, 4, 0);
SELECT_CQT_CALC(fma4, FMA4, 4, 0); 
SELECT_CQT_CALC(avx, AVX_FAST, 8, 01452367);
SELECT_CQT_CALC(fma3, FMA3_FAST, 8, 01452367);
}
