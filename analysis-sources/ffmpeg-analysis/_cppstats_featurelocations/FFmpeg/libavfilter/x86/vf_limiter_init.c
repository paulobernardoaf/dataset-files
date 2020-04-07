

















#include "libavutil/x86/cpu.h"

#include "libavfilter/limiter.h"

void ff_limiter_8bit_sse2(const uint8_t *src, uint8_t *dst,
ptrdiff_t slinesize, ptrdiff_t dlinesize,
int w, int h, int min, int max);
void ff_limiter_16bit_sse4(const uint8_t *src, uint8_t *dst,
ptrdiff_t slinesize, ptrdiff_t dlinesize,
int w, int h, int min, int max);

void ff_limiter_init_x86(LimiterDSPContext *dsp, int bpp)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags)) {
if (bpp <= 8) {
dsp->limiter = ff_limiter_8bit_sse2;
}
}
if (EXTERNAL_SSE4(cpu_flags)) {
if (bpp > 8) {
dsp->limiter = ff_limiter_16bit_sse4;
}
}
}
