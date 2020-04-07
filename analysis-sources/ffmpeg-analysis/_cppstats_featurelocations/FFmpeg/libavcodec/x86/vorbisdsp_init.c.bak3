



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/vorbisdsp.h"

void ff_vorbis_inverse_coupling_3dnow(float *mag, float *ang,
intptr_t blocksize);
void ff_vorbis_inverse_coupling_sse(float *mag, float *ang,
intptr_t blocksize);

av_cold void ff_vorbisdsp_init_x86(VorbisDSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();

#if ARCH_X86_32
if (EXTERNAL_AMD3DNOW(cpu_flags))
dsp->vorbis_inverse_coupling = ff_vorbis_inverse_coupling_3dnow;
#endif 
if (EXTERNAL_SSE(cpu_flags))
dsp->vorbis_inverse_coupling = ff_vorbis_inverse_coupling_sse;
}
