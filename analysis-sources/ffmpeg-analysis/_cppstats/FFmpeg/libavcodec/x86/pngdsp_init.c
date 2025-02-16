#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/pngdsp.h"
void ff_add_png_paeth_prediction_mmxext(uint8_t *dst, uint8_t *src,
uint8_t *top, int w, int bpp);
void ff_add_png_paeth_prediction_ssse3(uint8_t *dst, uint8_t *src,
uint8_t *top, int w, int bpp);
void ff_add_bytes_l2_mmx (uint8_t *dst, uint8_t *src1,
uint8_t *src2, int w);
void ff_add_bytes_l2_sse2(uint8_t *dst, uint8_t *src1,
uint8_t *src2, int w);
av_cold void ff_pngdsp_init_x86(PNGDSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();
#if ARCH_X86_32
if (EXTERNAL_MMX(cpu_flags))
dsp->add_bytes_l2 = ff_add_bytes_l2_mmx;
#endif
if (EXTERNAL_MMXEXT(cpu_flags))
dsp->add_paeth_prediction = ff_add_png_paeth_prediction_mmxext;
if (EXTERNAL_SSE2(cpu_flags))
dsp->add_bytes_l2 = ff_add_bytes_l2_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
dsp->add_paeth_prediction = ff_add_png_paeth_prediction_ssse3;
}
