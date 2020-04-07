

















#include "config.h"

#include "libavutil/aarch64/cpu.h"
#include "libavcodec/aacpsdsp.h"

void ff_ps_add_squares_neon(float *dst, const float (*src)[2], int n);
void ff_ps_mul_pair_single_neon(float (*dst)[2], float (*src0)[2],
float *src1, int n);
void ff_ps_hybrid_analysis_neon(float (*out)[2], float (*in)[2],
const float (*filter)[8][2],
ptrdiff_t stride, int n);
void ff_ps_stereo_interpolate_neon(float (*l)[2], float (*r)[2],
float h[2][4], float h_step[2][4],
int len);
void ff_ps_stereo_interpolate_ipdopd_neon(float (*l)[2], float (*r)[2],
float h[2][4], float h_step[2][4],
int len);

av_cold void ff_psdsp_init_aarch64(PSDSPContext *s)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags)) {
s->add_squares = ff_ps_add_squares_neon;
s->mul_pair_single = ff_ps_mul_pair_single_neon;
s->hybrid_analysis = ff_ps_hybrid_analysis_neon;
s->stereo_interpolate[0] = ff_ps_stereo_interpolate_neon;
s->stereo_interpolate[1] = ff_ps_stereo_interpolate_ipdopd_neon;
}
}
