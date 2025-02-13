

















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/arm/startcode.h"
#include "libavcodec/vc1dsp.h"
#include "vc1dsp.h"

av_cold void ff_vc1dsp_init_arm(VC1DSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();

#if HAVE_ARMV6
if (have_setend(cpu_flags))
dsp->startcode_find_candidate = ff_startcode_find_candidate_armv6;
#endif
if (have_neon(cpu_flags))
ff_vc1dsp_init_neon(dsp);
}
