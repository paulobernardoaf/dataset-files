




















































#if !defined(AVCODEC_MIPS_LSP_MIPS_H)
#define AVCODEC_MIPS_LSP_MIPS_H

#if HAVE_MIPSFPU && HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
#include "libavutil/mips/asmdefs.h"

static av_always_inline void ff_lsp2polyf_mips(const double *lsp, double *f, int lp_half_order)
{
int i, j = 0;
double * p_fi = f;
double * p_f = 0;

f[0] = 1.0;
f[1] = -2 * lsp[0];
lsp -= 2;

for(i=2; i<=lp_half_order; i++)
{
double tmp, f_j_2, f_j_1, f_j;
double val = lsp[2*i];

__asm__ volatile(
"move %[p_f], %[p_fi] \n\t"
"add.d %[val], %[val], %[val] \n\t"
PTR_ADDIU "%[p_fi], 8 \n\t"
"ldc1 %[f_j_1], 0(%[p_f]) \n\t"
"ldc1 %[f_j], 8(%[p_f]) \n\t"
"neg.d %[val], %[val] \n\t"
"add.d %[tmp], %[f_j_1], %[f_j_1] \n\t"
"madd.d %[tmp], %[tmp], %[f_j], %[val] \n\t"
"addiu %[j], %[i], -2 \n\t"
"ldc1 %[f_j_2], -8(%[p_f]) \n\t"
"sdc1 %[tmp], 16(%[p_f]) \n\t"
"beqz %[j], ff_lsp2polyf_lp_j_end%= \n\t"
"ff_lsp2polyf_lp_j%=: \n\t"
"add.d %[tmp], %[f_j], %[f_j_2] \n\t"
"madd.d %[tmp], %[tmp], %[f_j_1], %[val] \n\t"
"mov.d %[f_j], %[f_j_1] \n\t"
"addiu %[j], -1 \n\t"
"mov.d %[f_j_1], %[f_j_2] \n\t"
"ldc1 %[f_j_2], -16(%[p_f]) \n\t"
"sdc1 %[tmp], 8(%[p_f]) \n\t"
PTR_ADDIU "%[p_f], -8 \n\t"
"bgtz %[j], ff_lsp2polyf_lp_j%= \n\t"
"ff_lsp2polyf_lp_j_end%=: \n\t"

: [f_j_2]"=&f"(f_j_2), [f_j_1]"=&f"(f_j_1), [val]"+f"(val),
[tmp]"=&f"(tmp), [f_j]"=&f"(f_j), [p_f]"+r"(p_f),
[j]"+r"(j), [p_fi]"+r"(p_fi)
: [i]"r"(i)
: "memory"
);
f[1] += val;
}
}
#define ff_lsp2polyf ff_lsp2polyf_mips
#endif 
#endif 
#endif 
