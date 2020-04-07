#include "config.h"
#include "libavutil/aarch64/cpu.h"
#include "libavcodec/opusdsp.h"
void ff_opus_postfilter_neon(float *data, int period, float *gains, int len);
float ff_opus_deemphasis_neon(float *out, float *in, float coeff, int len);
av_cold void ff_opus_dsp_init_aarch64(OpusDSP *ctx)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags)) {
ctx->postfilter = ff_opus_postfilter_neon;
ctx->deemphasis = ff_opus_deemphasis_neon;
}
}
