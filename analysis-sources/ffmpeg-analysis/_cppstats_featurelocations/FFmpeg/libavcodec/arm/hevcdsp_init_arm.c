



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"

#include "libavcodec/hevcdsp.h"
#include "hevcdsp_arm.h"

av_cold void ff_hevc_dsp_init_arm(HEVCDSPContext *c, const int bit_depth)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags))
ff_hevc_dsp_init_neon(c, bit_depth);
}
