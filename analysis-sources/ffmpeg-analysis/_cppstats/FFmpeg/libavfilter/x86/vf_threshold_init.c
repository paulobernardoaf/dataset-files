#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/threshold.h"
#define THRESHOLD_FUNC(depth, opt) void ff_threshold##depth##_##opt(const uint8_t *in, const uint8_t *threshold,const uint8_t *min, const uint8_t *max, uint8_t *out, ptrdiff_t ilinesize, ptrdiff_t tlinesize, ptrdiff_t flinesize, ptrdiff_t slinesize, ptrdiff_t olinesize, int w, int h);
THRESHOLD_FUNC(8, sse4)
THRESHOLD_FUNC(8, avx2)
THRESHOLD_FUNC(16, sse4)
THRESHOLD_FUNC(16, avx2)
av_cold void ff_threshold_init_x86(ThresholdContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (s->depth == 8) {
if (EXTERNAL_SSE4(cpu_flags)) {
s->threshold = ff_threshold8_sse4;
}
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
s->threshold = ff_threshold8_avx2;
}
} else if (s->depth == 16) {
if (EXTERNAL_SSE4(cpu_flags)) {
s->threshold = ff_threshold16_sse4;
}
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
s->threshold = ff_threshold16_avx2;
}
}
}
