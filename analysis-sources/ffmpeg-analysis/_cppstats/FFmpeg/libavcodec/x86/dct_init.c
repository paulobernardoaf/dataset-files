#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/dct.h"
void ff_dct32_float_sse(FFTSample *out, const FFTSample *in);
void ff_dct32_float_sse2(FFTSample *out, const FFTSample *in);
void ff_dct32_float_avx(FFTSample *out, const FFTSample *in);
av_cold void ff_dct_init_x86(DCTContext *s)
{
int cpu_flags = av_get_cpu_flags();
#if ARCH_X86_32
if (EXTERNAL_SSE(cpu_flags))
s->dct32 = ff_dct32_float_sse;
#endif
if (EXTERNAL_SSE2(cpu_flags))
s->dct32 = ff_dct32_float_sse2;
if (EXTERNAL_AVX_FAST(cpu_flags))
s->dct32 = ff_dct32_float_avx;
}
