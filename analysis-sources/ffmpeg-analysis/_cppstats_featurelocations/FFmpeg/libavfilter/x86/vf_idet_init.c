

















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/vf_idet.h"

#if HAVE_X86ASM


#define FUNC_MAIN_DECL(KIND, SPAN) int ff_idet_filter_line_##KIND(const uint8_t *a, const uint8_t *b, const uint8_t *c, int w); static int idet_filter_line_##KIND(const uint8_t *a, const uint8_t *b, const uint8_t *c, int w) { int sum = 0; const int left_over = w & (SPAN - 1); w -= left_over; if (w > 0) sum += ff_idet_filter_line_##KIND(a, b, c, w); if (left_over > 0) sum += ff_idet_filter_line_c(a + w, b + w, c + w, left_over); return sum; }















#define FUNC_MAIN_DECL_16bit(KIND, SPAN) int ff_idet_filter_line_16bit_##KIND(const uint16_t *a, const uint16_t *b, const uint16_t *c, int w); static int idet_filter_line_16bit_##KIND(const uint16_t *a, const uint16_t *b, const uint16_t *c, int w) { int sum = 0; const int left_over = w & (SPAN - 1); w -= left_over; if (w > 0) sum += ff_idet_filter_line_16bit_##KIND(a, b, c, w); if (left_over > 0) sum += ff_idet_filter_line_c_16bit(a + w, b + w, c + w, left_over); return sum; }














FUNC_MAIN_DECL(sse2, 16)
FUNC_MAIN_DECL_16bit(sse2, 8)
#if ARCH_X86_32
FUNC_MAIN_DECL(mmx, 8)
FUNC_MAIN_DECL(mmxext, 8)
FUNC_MAIN_DECL_16bit(mmx, 4)
#endif

#endif
av_cold void ff_idet_init_x86(IDETContext *idet, int for_16b)
{
#if HAVE_X86ASM
const int cpu_flags = av_get_cpu_flags();

#if ARCH_X86_32
if (EXTERNAL_MMX(cpu_flags)) {
idet->filter_line = for_16b ? (ff_idet_filter_func)idet_filter_line_16bit_mmx : idet_filter_line_mmx;
}
if (EXTERNAL_MMXEXT(cpu_flags)) {
idet->filter_line = for_16b ? (ff_idet_filter_func)idet_filter_line_16bit_mmx : idet_filter_line_mmxext;
}
#endif 

if (EXTERNAL_SSE2(cpu_flags)) {
idet->filter_line = for_16b ? (ff_idet_filter_func)idet_filter_line_16bit_sse2 : idet_filter_line_sse2;
}
#endif 
}
