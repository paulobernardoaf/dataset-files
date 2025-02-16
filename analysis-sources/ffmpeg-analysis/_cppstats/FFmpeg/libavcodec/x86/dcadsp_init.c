#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/dcadsp.h"
#define LFE_FIR_FLOAT_FUNC(opt) void ff_lfe_fir0_float_##opt(float *pcm_samples, int32_t *lfe_samples, const float *filter_coeff, ptrdiff_t npcmblocks); void ff_lfe_fir1_float_##opt(float *pcm_samples, int32_t *lfe_samples, const float *filter_coeff, ptrdiff_t npcmblocks);
LFE_FIR_FLOAT_FUNC(sse)
LFE_FIR_FLOAT_FUNC(sse2)
LFE_FIR_FLOAT_FUNC(sse3)
LFE_FIR_FLOAT_FUNC(avx)
LFE_FIR_FLOAT_FUNC(fma3)
av_cold void ff_dcadsp_init_x86(DCADSPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (ARCH_X86_32 && EXTERNAL_SSE(cpu_flags))
s->lfe_fir_float[0] = ff_lfe_fir0_float_sse;
if (EXTERNAL_SSE2(cpu_flags))
s->lfe_fir_float[0] = ff_lfe_fir0_float_sse2;
if (EXTERNAL_SSE3(cpu_flags))
s->lfe_fir_float[1] = ff_lfe_fir1_float_sse3;
if (EXTERNAL_AVX(cpu_flags)) {
s->lfe_fir_float[0] = ff_lfe_fir0_float_avx;
s->lfe_fir_float[1] = ff_lfe_fir1_float_avx;
}
if (EXTERNAL_FMA3(cpu_flags))
s->lfe_fir_float[0] = ff_lfe_fir0_float_fma3;
}
