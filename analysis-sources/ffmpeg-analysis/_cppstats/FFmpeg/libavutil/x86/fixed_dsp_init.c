#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/fixed_dsp.h"
#include "cpu.h"
void ff_butterflies_fixed_sse2(int *src0, int *src1, int len);
av_cold void ff_fixed_dsp_init_x86(AVFixedDSPContext *fdsp)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags)) {
fdsp->butterflies_fixed = ff_butterflies_fixed_sse2;
}
}
