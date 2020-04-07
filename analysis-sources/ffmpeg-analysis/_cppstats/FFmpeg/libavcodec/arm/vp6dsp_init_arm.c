#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/vp56dsp.h"
void ff_vp6_edge_filter_hor_neon(uint8_t *yuv, ptrdiff_t stride, int t);
void ff_vp6_edge_filter_ver_neon(uint8_t *yuv, ptrdiff_t stride, int t);
av_cold void ff_vp6dsp_init_arm(VP56DSPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags)) {
s->edge_filter_hor = ff_vp6_edge_filter_hor_neon;
s->edge_filter_ver = ff_vp6_edge_filter_ver_neon;
}
}
