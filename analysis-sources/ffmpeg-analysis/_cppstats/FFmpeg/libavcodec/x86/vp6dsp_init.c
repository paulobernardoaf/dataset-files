#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/vp56dsp.h"
void ff_vp6_filter_diag4_mmx(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
const int16_t *h_weights,const int16_t *v_weights);
void ff_vp6_filter_diag4_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
const int16_t *h_weights,const int16_t *v_weights);
av_cold void ff_vp6dsp_init_x86(VP56DSPContext *c)
{
int cpu_flags = av_get_cpu_flags();
#if ARCH_X86_32
if (EXTERNAL_MMX(cpu_flags)) {
c->vp6_filter_diag4 = ff_vp6_filter_diag4_mmx;
}
#endif
if (EXTERNAL_SSE2(cpu_flags)) {
c->vp6_filter_diag4 = ff_vp6_filter_diag4_sse2;
}
}
