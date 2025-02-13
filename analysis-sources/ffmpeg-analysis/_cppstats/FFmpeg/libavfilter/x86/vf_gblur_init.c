#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/gblur.h"
void ff_horiz_slice_sse4(float *ptr, int width, int height, int steps, float nu, float bscale);
void ff_horiz_slice_avx2(float *ptr, int width, int height, int steps, float nu, float bscale);
av_cold void ff_gblur_init_x86(GBlurContext *s)
{
#if ARCH_X86_64
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE4(cpu_flags))
s->horiz_slice = ff_horiz_slice_sse4;
if (EXTERNAL_AVX2(cpu_flags))
s->horiz_slice = ff_horiz_slice_avx2;
#endif
}
