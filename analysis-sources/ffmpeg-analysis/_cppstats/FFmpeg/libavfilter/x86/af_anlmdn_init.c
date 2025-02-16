#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/af_anlmdndsp.h"
float ff_compute_distance_ssd_sse(const float *f1, const float *f2,
ptrdiff_t len);
av_cold void ff_anlmdn_init_x86(AudioNLMDNDSPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE(cpu_flags)) {
s->compute_distance_ssd = ff_compute_distance_ssd_sse;
}
}
