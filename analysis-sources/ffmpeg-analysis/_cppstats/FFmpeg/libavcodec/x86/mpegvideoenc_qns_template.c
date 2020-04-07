#include <stdint.h>
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/x86/asm.h"
#include "inline_asm.h"
#define MAX_ABS (512 >> (SCALE_OFFSET>0 ? SCALE_OFFSET : 0))
static int DEF(try_8x8basis)(int16_t rem[64], int16_t weight[64], int16_t basis[64], int scale)
{
x86_reg i=0;
av_assert2(FFABS(scale) < MAX_ABS);
scale<<= 16 + SCALE_OFFSET - BASIS_SHIFT + RECON_SHIFT;
SET_RND(mm6);
__asm__ volatile(
"pxor %%mm7, %%mm7 \n\t"
"movd %4, %%mm5 \n\t"
"punpcklwd %%mm5, %%mm5 \n\t"
"punpcklwd %%mm5, %%mm5 \n\t"
".p2align 4 \n\t"
"1: \n\t"
"movq (%1, %0), %%mm0 \n\t"
"movq 8(%1, %0), %%mm1 \n\t"
PMULHRW(%%mm0, %%mm1, %%mm5, %%mm6)
"paddw (%2, %0), %%mm0 \n\t"
"paddw 8(%2, %0), %%mm1 \n\t"
"psraw $6, %%mm0 \n\t"
"psraw $6, %%mm1 \n\t"
"pmullw (%3, %0), %%mm0 \n\t"
"pmullw 8(%3, %0), %%mm1 \n\t"
"pmaddwd %%mm0, %%mm0 \n\t"
"pmaddwd %%mm1, %%mm1 \n\t"
"paddd %%mm1, %%mm0 \n\t"
"psrld $4, %%mm0 \n\t"
"paddd %%mm0, %%mm7 \n\t"
"add $16, %0 \n\t"
"cmp $128, %0 \n\t" 
" jb 1b \n\t"
PHADDD(%%mm7, %%mm6)
"psrld $2, %%mm7 \n\t"
"movd %%mm7, %0 \n\t"
: "+r" (i)
: "r"(basis), "r"(rem), "r"(weight), "g"(scale)
);
return i;
}
static void DEF(add_8x8basis)(int16_t rem[64], int16_t basis[64], int scale)
{
x86_reg i=0;
if(FFABS(scale) < MAX_ABS){
scale<<= 16 + SCALE_OFFSET - BASIS_SHIFT + RECON_SHIFT;
SET_RND(mm6);
__asm__ volatile(
"movd %3, %%mm5 \n\t"
"punpcklwd %%mm5, %%mm5 \n\t"
"punpcklwd %%mm5, %%mm5 \n\t"
".p2align 4 \n\t"
"1: \n\t"
"movq (%1, %0), %%mm0 \n\t"
"movq 8(%1, %0), %%mm1 \n\t"
PMULHRW(%%mm0, %%mm1, %%mm5, %%mm6)
"paddw (%2, %0), %%mm0 \n\t"
"paddw 8(%2, %0), %%mm1 \n\t"
"movq %%mm0, (%2, %0) \n\t"
"movq %%mm1, 8(%2, %0) \n\t"
"add $16, %0 \n\t"
"cmp $128, %0 \n\t" 
" jb 1b \n\t"
: "+r" (i)
: "r"(basis), "r"(rem), "g"(scale)
);
}else{
for(i=0; i<8*8; i++){
rem[i] += (basis[i]*scale + (1<<(BASIS_SHIFT - RECON_SHIFT-1)))>>(BASIS_SHIFT - RECON_SHIFT);
}
}
}
