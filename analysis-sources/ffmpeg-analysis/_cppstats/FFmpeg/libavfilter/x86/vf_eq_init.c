#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/cpu.h"
#include "libavutil/x86/asm.h"
#include "libavfilter/vf_eq.h"
extern void ff_process_one_line_mmxext(const uint8_t *src, uint8_t *dst, short contrast,
short brightness, int w);
extern void ff_process_one_line_sse2(const uint8_t *src, uint8_t *dst, short contrast,
short brightness, int w);
#if HAVE_X86ASM
static void process_mmxext(EQParameters *param, uint8_t *dst, int dst_stride,
const uint8_t *src, int src_stride, int w, int h)
{
short contrast = (short) (param->contrast * 256 * 16);
short brightness = ((short) (100.0 * param->brightness + 100.0) * 511)
/ 200 - 128 - contrast / 32;
while (h--) {
ff_process_one_line_mmxext(src, dst, contrast, brightness, w);
src += src_stride;
dst += dst_stride;
}
emms_c();
}
static void process_sse2(EQParameters *param, uint8_t *dst, int dst_stride,
const uint8_t *src, int src_stride, int w, int h)
{
short contrast = (short) (param->contrast * 256 * 16);
short brightness = ((short) (100.0 * param->brightness + 100.0) * 511)
/ 200 - 128 - contrast / 32;
while (h--) {
ff_process_one_line_sse2(src, dst, contrast, brightness, w);
src += src_stride;
dst += dst_stride;
}
}
#endif
av_cold void ff_eq_init_x86(EQContext *eq)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_MMXEXT(cpu_flags)) {
eq->process = process_mmxext;
}
if (EXTERNAL_SSE2(cpu_flags)) {
eq->process = process_sse2;
}
#endif
}
