#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/g722dsp.h"
void ff_g722_apply_qmf_sse2(const int16_t *prev_samples, int xout[2]);
av_cold void ff_g722dsp_init_x86(G722DSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags))
dsp->apply_qmf = ff_g722_apply_qmf_sse2;
}
