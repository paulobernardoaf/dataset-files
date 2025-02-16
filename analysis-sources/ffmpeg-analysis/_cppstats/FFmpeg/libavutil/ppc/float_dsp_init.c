#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/float_dsp.h"
#include "libavutil/ppc/cpu.h"
#include "float_dsp_altivec.h"
#include "float_dsp_vsx.h"
av_cold void ff_float_dsp_init_ppc(AVFloatDSPContext *fdsp, int bit_exact)
{
if (PPC_ALTIVEC(av_get_cpu_flags())) {
fdsp->vector_fmul = ff_vector_fmul_altivec;
fdsp->vector_fmul_add = ff_vector_fmul_add_altivec;
fdsp->vector_fmul_reverse = ff_vector_fmul_reverse_altivec;
if (!bit_exact) {
fdsp->vector_fmul_window = ff_vector_fmul_window_altivec;
}
}
if (PPC_VSX(av_get_cpu_flags())) {
fdsp->vector_fmul_add = ff_vector_fmul_add_vsx;
}
}
