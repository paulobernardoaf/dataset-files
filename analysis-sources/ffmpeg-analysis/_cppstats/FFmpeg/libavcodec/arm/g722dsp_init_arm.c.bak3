



















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/g722dsp.h"

extern void ff_g722_apply_qmf_neon(const int16_t *prev_samples, int xout[2]);

av_cold void ff_g722dsp_init_arm(G722DSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags))
dsp->apply_qmf = ff_g722_apply_qmf_neon;
}
