#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/aarch64/cpu.h"
#include "libavcodec/videodsp.h"
void ff_prefetch_aarch64(uint8_t *mem, ptrdiff_t stride, int h);
av_cold void ff_videodsp_init_aarch64(VideoDSPContext *ctx, int bpc)
{
int cpu_flags = av_get_cpu_flags();
if (have_armv8(cpu_flags))
ctx->prefetch = ff_prefetch_aarch64;
}
