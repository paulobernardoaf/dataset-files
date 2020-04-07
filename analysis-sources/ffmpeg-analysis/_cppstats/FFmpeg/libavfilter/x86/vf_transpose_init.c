#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/transpose.h"
void ff_transpose_8x8_8_sse2(uint8_t *src,
ptrdiff_t src_linesize,
uint8_t *dst,
ptrdiff_t dst_linesize);
void ff_transpose_8x8_16_sse2(uint8_t *src,
ptrdiff_t src_linesize,
uint8_t *dst,
ptrdiff_t dst_linesize);
av_cold void ff_transpose_init_x86(TransVtable *v, int pixstep)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags) && pixstep == 1) {
v->transpose_8x8 = ff_transpose_8x8_8_sse2;
}
if (EXTERNAL_SSE2(cpu_flags) && pixstep == 2) {
v->transpose_8x8 = ff_transpose_8x8_16_sse2;
}
}
