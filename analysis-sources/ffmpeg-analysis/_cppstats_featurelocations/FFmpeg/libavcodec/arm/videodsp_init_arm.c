



















#include "libavutil/attributes.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/videodsp.h"
#include "videodsp_arm.h"

av_cold void ff_videodsp_init_arm(VideoDSPContext *ctx, int bpc)
{
int cpu_flags = av_get_cpu_flags();
if (have_armv5te(cpu_flags)) ff_videodsp_init_armv5te(ctx, bpc);
}
