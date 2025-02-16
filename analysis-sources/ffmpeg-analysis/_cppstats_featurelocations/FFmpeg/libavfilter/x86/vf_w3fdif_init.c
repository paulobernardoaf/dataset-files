



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/w3fdif.h"

void ff_w3fdif_simple_low_sse2(int32_t *work_line,
uint8_t *in_lines_cur[2],
const int16_t *coef, int linesize);

void ff_w3fdif_simple_high_sse2(int32_t *work_line,
uint8_t *in_lines_cur[3],
uint8_t *in_lines_adj[3],
const int16_t *coef, int linesize);

void ff_w3fdif_complex_low_sse2(int32_t *work_line,
uint8_t *in_lines_cur[4],
const int16_t *coef, int linesize);

void ff_w3fdif_complex_high_sse2(int32_t *work_line,
uint8_t *in_lines_cur[5],
uint8_t *in_lines_adj[5],
const int16_t *coef, int linesize);

void ff_w3fdif_scale_sse2(uint8_t *out_pixel, const int32_t *work_pixel,
int linesize, int max);

av_cold void ff_w3fdif_init_x86(W3FDIFDSPContext *dsp, int depth)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags) && depth <= 8) {
dsp->filter_simple_low = ff_w3fdif_simple_low_sse2;
dsp->filter_simple_high = ff_w3fdif_simple_high_sse2;
dsp->filter_complex_low = ff_w3fdif_complex_low_sse2;
dsp->filter_scale = ff_w3fdif_scale_sse2;
}

if (ARCH_X86_64 && EXTERNAL_SSE2(cpu_flags) && depth <= 8) {
dsp->filter_complex_high = ff_w3fdif_complex_high_sse2;
}
}
