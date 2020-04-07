#include "config.h"
#include "libavutil/attributes.h"
#include "vorbisdsp.h"
#include "vorbis.h"
av_cold void ff_vorbisdsp_init(VorbisDSPContext *dsp)
{
dsp->vorbis_inverse_coupling = ff_vorbis_inverse_coupling;
if (ARCH_AARCH64)
ff_vorbisdsp_init_aarch64(dsp);
if (ARCH_ARM)
ff_vorbisdsp_init_arm(dsp);
if (ARCH_PPC)
ff_vorbisdsp_init_ppc(dsp);
if (ARCH_X86)
ff_vorbisdsp_init_x86(dsp);
}
