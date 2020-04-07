



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/audiodsp.h"
#include "audiodsp_arm.h"

av_cold void ff_audiodsp_init_arm(AudioDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags))
ff_audiodsp_init_neon(c);
}
