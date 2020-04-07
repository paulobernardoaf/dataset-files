




















#include "config.h"

#include "libavutil/x86/cpu.h"
#include "libavcodec/opus_pvq.h"

extern float ff_pvq_search_approx_sse2(float *X, int *y, int K, int N);
extern float ff_pvq_search_approx_sse4(float *X, int *y, int K, int N);
extern float ff_pvq_search_exact_avx (float *X, int *y, int K, int N);

av_cold void ff_celt_pvq_init_x86(CeltPVQ *s)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags))
s->pvq_search = ff_pvq_search_approx_sse2;

if (EXTERNAL_SSE4(cpu_flags))
s->pvq_search = ff_pvq_search_approx_sse4;

if (EXTERNAL_AVX_FAST(cpu_flags))
s->pvq_search = ff_pvq_search_exact_avx;
}
