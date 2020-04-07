#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/rv34dsp.h"
void ff_rv34_idct_dc_mmxext(int16_t *block);
void ff_rv34_idct_dc_noround_mmxext(int16_t *block);
void ff_rv34_idct_dc_add_mmx(uint8_t *dst, ptrdiff_t stride, int dc);
void ff_rv34_idct_dc_add_sse2(uint8_t *dst, ptrdiff_t stride, int dc);
void ff_rv34_idct_dc_add_sse4(uint8_t *dst, ptrdiff_t stride, int dc);
void ff_rv34_idct_add_mmxext(uint8_t *dst, ptrdiff_t stride, int16_t *block);
av_cold void ff_rv34dsp_init_x86(RV34DSPContext* c)
{
int cpu_flags = av_get_cpu_flags();
if (ARCH_X86_32 && EXTERNAL_MMX(cpu_flags))
c->rv34_idct_dc_add = ff_rv34_idct_dc_add_mmx;
if (EXTERNAL_MMXEXT(cpu_flags)) {
c->rv34_inv_transform_dc = ff_rv34_idct_dc_noround_mmxext;
c->rv34_idct_add = ff_rv34_idct_add_mmxext;
}
if (EXTERNAL_SSE2(cpu_flags))
c->rv34_idct_dc_add = ff_rv34_idct_dc_add_sse2;
if (EXTERNAL_SSE4(cpu_flags))
c->rv34_idct_dc_add = ff_rv34_idct_dc_add_sse4;
}
