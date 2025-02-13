























#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/lossless_videoencdsp.h"
#include "libavcodec/mathops.h"

void ff_diff_bytes_mmx(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
intptr_t w);
void ff_diff_bytes_sse2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
intptr_t w);
void ff_diff_bytes_avx2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
intptr_t w);

void ff_sub_left_predict_avx(uint8_t *dst, uint8_t *src,
ptrdiff_t stride, ptrdiff_t width, int height);

#if HAVE_INLINE_ASM

static void sub_median_pred_mmxext(uint8_t *dst, const uint8_t *src1,
const uint8_t *src2, intptr_t w,
int *left, int *left_top)
{
x86_reg i = 0;
uint8_t l, lt;

__asm__ volatile (
"movq (%1, %0), %%mm0 \n\t" 
"psllq $8, %%mm0 \n\t"
"1: \n\t"
"movq (%1, %0), %%mm1 \n\t" 
"movq -1(%2, %0), %%mm2 \n\t" 
"movq (%2, %0), %%mm3 \n\t" 
"movq %%mm2, %%mm4 \n\t" 
"psubb %%mm0, %%mm2 \n\t"
"paddb %%mm1, %%mm2 \n\t" 
"movq %%mm4, %%mm5 \n\t" 
"pmaxub %%mm1, %%mm4 \n\t" 
"pminub %%mm5, %%mm1 \n\t" 
"pminub %%mm2, %%mm4 \n\t"
"pmaxub %%mm1, %%mm4 \n\t"
"psubb %%mm4, %%mm3 \n\t" 
"movq %%mm3, (%3, %0) \n\t"
"add $8, %0 \n\t"
"movq -1(%1, %0), %%mm0 \n\t" 
"cmp %4, %0 \n\t"
" jb 1b \n\t"
: "+r" (i)
: "r" (src1), "r" (src2), "r" (dst), "r" ((x86_reg) w));

l = *left;
lt = *left_top;

dst[0] = src2[0] - mid_pred(l, src1[0], (l + src1[0] - lt) & 0xFF);

*left_top = src1[w - 1];
*left = src2[w - 1];
}

#endif 

av_cold void ff_llvidencdsp_init_x86(LLVidEncDSPContext *c)
{
av_unused int cpu_flags = av_get_cpu_flags();

if (ARCH_X86_32 && EXTERNAL_MMX(cpu_flags)) {
c->diff_bytes = ff_diff_bytes_mmx;
}

#if HAVE_INLINE_ASM
if (INLINE_MMXEXT(cpu_flags)) {
c->sub_median_pred = sub_median_pred_mmxext;
}
#endif 

if (EXTERNAL_SSE2(cpu_flags)) {
c->diff_bytes = ff_diff_bytes_sse2;
}

if (EXTERNAL_AVX(cpu_flags)) {
c->sub_left_predict = ff_sub_left_predict_avx;
}

if (EXTERNAL_AVX2_FAST(cpu_flags)) {
c->diff_bytes = ff_diff_bytes_avx2;
}
}
