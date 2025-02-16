




















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/vorbisdsp.h"

void ff_vorbis_inverse_coupling_neon(float *mag, float *ang,
intptr_t blocksize);

av_cold void ff_vorbisdsp_init_arm(VorbisDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags)) {
c->vorbis_inverse_coupling = ff_vorbis_inverse_coupling_neon;
}
}
