#include <stddef.h>
#include <stdint.h>
#include "inline_asm.h"
av_unused STATIC void DEF(put, pixels8_xy2)(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h)
{
MOVQ_ZERO(mm7);
SET_RND(mm6); 
__asm__ volatile(
"movq (%1), %%mm0 \n\t"
"movq 1(%1), %%mm4 \n\t"
"movq %%mm0, %%mm1 \n\t"
"movq %%mm4, %%mm5 \n\t"
"punpcklbw %%mm7, %%mm0 \n\t"
"punpcklbw %%mm7, %%mm4 \n\t"
"punpckhbw %%mm7, %%mm1 \n\t"
"punpckhbw %%mm7, %%mm5 \n\t"
"paddusw %%mm0, %%mm4 \n\t"
"paddusw %%mm1, %%mm5 \n\t"
"xor %%"FF_REG_a", %%"FF_REG_a" \n\t"
"add %3, %1 \n\t"
".p2align 3 \n\t"
"1: \n\t"
"movq (%1, %%"FF_REG_a"), %%mm0 \n\t"
"movq 1(%1, %%"FF_REG_a"), %%mm2 \n\t"
"movq %%mm0, %%mm1 \n\t"
"movq %%mm2, %%mm3 \n\t"
"punpcklbw %%mm7, %%mm0 \n\t"
"punpcklbw %%mm7, %%mm2 \n\t"
"punpckhbw %%mm7, %%mm1 \n\t"
"punpckhbw %%mm7, %%mm3 \n\t"
"paddusw %%mm2, %%mm0 \n\t"
"paddusw %%mm3, %%mm1 \n\t"
"paddusw %%mm6, %%mm4 \n\t"
"paddusw %%mm6, %%mm5 \n\t"
"paddusw %%mm0, %%mm4 \n\t"
"paddusw %%mm1, %%mm5 \n\t"
"psrlw $2, %%mm4 \n\t"
"psrlw $2, %%mm5 \n\t"
"packuswb %%mm5, %%mm4 \n\t"
"movq %%mm4, (%2, %%"FF_REG_a") \n\t"
"add %3, %%"FF_REG_a" \n\t"
"movq (%1, %%"FF_REG_a"), %%mm2 \n\t" 
"movq 1(%1, %%"FF_REG_a"), %%mm4 \n\t"
"movq %%mm2, %%mm3 \n\t"
"movq %%mm4, %%mm5 \n\t"
"punpcklbw %%mm7, %%mm2 \n\t"
"punpcklbw %%mm7, %%mm4 \n\t"
"punpckhbw %%mm7, %%mm3 \n\t"
"punpckhbw %%mm7, %%mm5 \n\t"
"paddusw %%mm2, %%mm4 \n\t"
"paddusw %%mm3, %%mm5 \n\t"
"paddusw %%mm6, %%mm0 \n\t"
"paddusw %%mm6, %%mm1 \n\t"
"paddusw %%mm4, %%mm0 \n\t"
"paddusw %%mm5, %%mm1 \n\t"
"psrlw $2, %%mm0 \n\t"
"psrlw $2, %%mm1 \n\t"
"packuswb %%mm1, %%mm0 \n\t"
"movq %%mm0, (%2, %%"FF_REG_a") \n\t"
"add %3, %%"FF_REG_a" \n\t"
"subl $2, %0 \n\t"
"jnz 1b \n\t"
:"+g"(h), "+S"(pixels)
:"D"(block), "r"((x86_reg)line_size)
:FF_REG_a, "memory");
}
av_unused STATIC void DEF(avg, pixels8_xy2)(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h)
{
MOVQ_ZERO(mm7);
SET_RND(mm6); 
__asm__ volatile(
"movq (%1), %%mm0 \n\t"
"movq 1(%1), %%mm4 \n\t"
"movq %%mm0, %%mm1 \n\t"
"movq %%mm4, %%mm5 \n\t"
"punpcklbw %%mm7, %%mm0 \n\t"
"punpcklbw %%mm7, %%mm4 \n\t"
"punpckhbw %%mm7, %%mm1 \n\t"
"punpckhbw %%mm7, %%mm5 \n\t"
"paddusw %%mm0, %%mm4 \n\t"
"paddusw %%mm1, %%mm5 \n\t"
"xor %%"FF_REG_a", %%"FF_REG_a" \n\t"
"add %3, %1 \n\t"
".p2align 3 \n\t"
"1: \n\t"
"movq (%1, %%"FF_REG_a"), %%mm0 \n\t"
"movq 1(%1, %%"FF_REG_a"), %%mm2 \n\t"
"movq %%mm0, %%mm1 \n\t"
"movq %%mm2, %%mm3 \n\t"
"punpcklbw %%mm7, %%mm0 \n\t"
"punpcklbw %%mm7, %%mm2 \n\t"
"punpckhbw %%mm7, %%mm1 \n\t"
"punpckhbw %%mm7, %%mm3 \n\t"
"paddusw %%mm2, %%mm0 \n\t"
"paddusw %%mm3, %%mm1 \n\t"
"paddusw %%mm6, %%mm4 \n\t"
"paddusw %%mm6, %%mm5 \n\t"
"paddusw %%mm0, %%mm4 \n\t"
"paddusw %%mm1, %%mm5 \n\t"
"psrlw $2, %%mm4 \n\t"
"psrlw $2, %%mm5 \n\t"
"movq (%2, %%"FF_REG_a"), %%mm3 \n\t"
"packuswb %%mm5, %%mm4 \n\t"
"pcmpeqd %%mm2, %%mm2 \n\t"
"paddb %%mm2, %%mm2 \n\t"
PAVGB_MMX(%%mm3, %%mm4, %%mm5, %%mm2)
"movq %%mm5, (%2, %%"FF_REG_a") \n\t"
"add %3, %%"FF_REG_a" \n\t"
"movq (%1, %%"FF_REG_a"), %%mm2 \n\t" 
"movq 1(%1, %%"FF_REG_a"), %%mm4 \n\t"
"movq %%mm2, %%mm3 \n\t"
"movq %%mm4, %%mm5 \n\t"
"punpcklbw %%mm7, %%mm2 \n\t"
"punpcklbw %%mm7, %%mm4 \n\t"
"punpckhbw %%mm7, %%mm3 \n\t"
"punpckhbw %%mm7, %%mm5 \n\t"
"paddusw %%mm2, %%mm4 \n\t"
"paddusw %%mm3, %%mm5 \n\t"
"paddusw %%mm6, %%mm0 \n\t"
"paddusw %%mm6, %%mm1 \n\t"
"paddusw %%mm4, %%mm0 \n\t"
"paddusw %%mm5, %%mm1 \n\t"
"psrlw $2, %%mm0 \n\t"
"psrlw $2, %%mm1 \n\t"
"movq (%2, %%"FF_REG_a"), %%mm3 \n\t"
"packuswb %%mm1, %%mm0 \n\t"
"pcmpeqd %%mm2, %%mm2 \n\t"
"paddb %%mm2, %%mm2 \n\t"
PAVGB_MMX(%%mm3, %%mm0, %%mm1, %%mm2)
"movq %%mm1, (%2, %%"FF_REG_a") \n\t"
"add %3, %%"FF_REG_a" \n\t"
"subl $2, %0 \n\t"
"jnz 1b \n\t"
:"+g"(h), "+S"(pixels)
:"D"(block), "r"((x86_reg)line_size)
:FF_REG_a, "memory");
}
