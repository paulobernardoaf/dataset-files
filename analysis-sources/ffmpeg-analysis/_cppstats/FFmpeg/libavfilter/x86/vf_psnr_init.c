#include "libavutil/x86/cpu.h"
#include "libavfilter/psnr.h"
uint64_t ff_sse_line_8bit_sse2(const uint8_t *buf, const uint8_t *ref, int w);
uint64_t ff_sse_line_16bit_sse2(const uint8_t *buf, const uint8_t *ref, int w);
void ff_psnr_init_x86(PSNRDSPContext *dsp, int bpp)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags)) {
if (bpp <= 8) {
dsp->sse_line = ff_sse_line_8bit_sse2;
} else if (bpp <= 15) {
dsp->sse_line = ff_sse_line_16bit_sse2;
}
}
}
