



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/blockdsp.h"
#include "blockdsp_arm.h"

av_cold void ff_blockdsp_init_arm(BlockDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags))
ff_blockdsp_init_neon(c);
}
