#include "libavutil/x86/cpu.h"
#include "libavfilter/stereo3d.h"
void ff_anaglyph_sse4(uint8_t *dst, uint8_t *lsrc, uint8_t *rsrc,
ptrdiff_t dst_linesize, ptrdiff_t l_linesize, ptrdiff_t r_linesize,
int width, int height,
const int *ana_matrix_r, const int *ana_matrix_g, const int *ana_matrix_b);
void ff_stereo3d_init_x86(Stereo3DDSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE4(cpu_flags)) {
dsp->anaglyph = ff_anaglyph_sse4;
}
}
