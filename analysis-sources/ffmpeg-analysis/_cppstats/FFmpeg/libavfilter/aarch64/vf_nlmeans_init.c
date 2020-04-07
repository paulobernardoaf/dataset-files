#include "libavutil/aarch64/cpu.h"
#include "libavfilter/vf_nlmeans.h"
void ff_compute_safe_ssd_integral_image_neon(uint32_t *dst, ptrdiff_t dst_linesize_32,
const uint8_t *s1, ptrdiff_t linesize1,
const uint8_t *s2, ptrdiff_t linesize2,
int w, int h);
av_cold void ff_nlmeans_init_aarch64(NLMeansDSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags))
dsp->compute_safe_ssd_integral_image = ff_compute_safe_ssd_integral_image_neon;
}
