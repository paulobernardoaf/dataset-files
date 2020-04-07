#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/mpegaudiodsp.h"
#include "config.h"
void ff_mpadsp_apply_window_fixed_armv6(int32_t *synth_buf, int32_t *window,
int *dither, int16_t *out, ptrdiff_t incr);
av_cold void ff_mpadsp_init_arm(MPADSPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (have_armv6(cpu_flags)) {
s->apply_window_fixed = ff_mpadsp_apply_window_fixed_armv6;
}
}
