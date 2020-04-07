#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/maskedmerge.h"
void ff_maskedmerge8_sse2(const uint8_t *bsrc, const uint8_t *osrc,
const uint8_t *msrc, uint8_t *dst,
ptrdiff_t blinesize, ptrdiff_t olinesize,
ptrdiff_t mlinesize, ptrdiff_t dlinesize,
int w, int h,
int half, int shift);
av_cold void ff_maskedmerge_init_x86(MaskedMergeContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags) && s->depth == 8) {
s->maskedmerge = ff_maskedmerge8_sse2;
}
}
