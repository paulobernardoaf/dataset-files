

















#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/alacdsp.h"
#include "config.h"

void ff_alac_decorrelate_stereo_sse4(int32_t *buffer[2], int nb_samples,
int decorr_shift, int decorr_left_weight);
void ff_alac_append_extra_bits_stereo_sse2(int32_t *buffer[2], int32_t *extra_bits_buffer[2],
int extra_bits, int channels, int nb_samples);
void ff_alac_append_extra_bits_mono_sse2(int32_t *buffer[2], int32_t *extra_bits_buffer[2],
int extra_bits, int channels, int nb_samples);

av_cold void ff_alacdsp_init_x86(ALACDSPContext *c)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags)) {
c->append_extra_bits[0] = ff_alac_append_extra_bits_mono_sse2;
c->append_extra_bits[1] = ff_alac_append_extra_bits_stereo_sse2;
}
if (EXTERNAL_SSE4(cpu_flags)) {
c->decorrelate_stereo = ff_alac_decorrelate_stereo_sse4;
}
#endif 
}
