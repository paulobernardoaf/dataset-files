


















































#include "libavutil/avutil.h"
#include "libavcodec/amrwbdata.h"
#include "amrwbdec_mips.h"

#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
void ff_hb_fir_filter_mips(float *out, const float fir_coef[HB_FIR_SIZE + 1],
float mem[HB_FIR_SIZE], const float *in)
{
int i;
float data[AMRWB_SFR_SIZE_16k + HB_FIR_SIZE]; 

memcpy(data, mem, HB_FIR_SIZE * sizeof(float));
memcpy(data + HB_FIR_SIZE, in, AMRWB_SFR_SIZE_16k * sizeof(float));

for (i = 0; i < AMRWB_SFR_SIZE_16k; i++) {
float output;
float * p_data = (data+i);





__asm__ volatile(
"mtc1 $zero, %[output] \n\t"
"lwc1 $f0, 0(%[p_data]) \n\t"
"lwc1 $f1, 0(%[fir_coef]) \n\t"
"lwc1 $f2, 4(%[p_data]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f3, 4(%[fir_coef]) \n\t"
"lwc1 $f4, 8(%[p_data]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"
"lwc1 $f5, 8(%[fir_coef]) \n\t"

"lwc1 $f0, 12(%[p_data]) \n\t"
"lwc1 $f1, 12(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f2, 16(%[p_data]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f3, 16(%[fir_coef]) \n\t"
"lwc1 $f4, 20(%[p_data]) \n\t"
"lwc1 $f5, 20(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 24(%[p_data]) \n\t"
"lwc1 $f1, 24(%[fir_coef]) \n\t"
"lwc1 $f2, 28(%[p_data]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f3, 28(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f4, 32(%[p_data]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"
"lwc1 $f5, 32(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"

"lwc1 $f0, 36(%[p_data]) \n\t"
"lwc1 $f1, 36(%[fir_coef]) \n\t"
"lwc1 $f2, 40(%[p_data]) \n\t"
"lwc1 $f3, 40(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f4, 44(%[p_data]) \n\t"
"lwc1 $f5, 44(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 48(%[p_data]) \n\t"
"lwc1 $f1, 48(%[fir_coef]) \n\t"
"lwc1 $f2, 52(%[p_data]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f3, 52(%[fir_coef]) \n\t"
"lwc1 $f4, 56(%[p_data]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f5, 56(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 60(%[p_data]) \n\t"
"lwc1 $f1, 60(%[fir_coef]) \n\t"
"lwc1 $f2, 64(%[p_data]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f3, 64(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f4, 68(%[p_data]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"
"lwc1 $f5, 68(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"

"lwc1 $f0, 72(%[p_data]) \n\t"
"lwc1 $f1, 72(%[fir_coef]) \n\t"
"lwc1 $f2, 76(%[p_data]) \n\t"
"lwc1 $f3, 76(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f4, 80(%[p_data]) \n\t"
"lwc1 $f5, 80(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 84(%[p_data]) \n\t"
"lwc1 $f1, 84(%[fir_coef]) \n\t"
"lwc1 $f2, 88(%[p_data]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f3, 88(%[fir_coef]) \n\t"
"lwc1 $f4, 92(%[p_data]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f5, 92(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 96(%[p_data]) \n\t"
"lwc1 $f1, 96(%[fir_coef]) \n\t"
"lwc1 $f2, 100(%[p_data]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f3, 100(%[fir_coef]) \n\t"
"lwc1 $f4, 104(%[p_data]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f5, 104(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"

"lwc1 $f0, 108(%[p_data]) \n\t"
"lwc1 $f1, 108(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"lwc1 $f2, 112(%[p_data]) \n\t"
"lwc1 $f3, 112(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"
"lwc1 $f4, 116(%[p_data]) \n\t"
"lwc1 $f5, 116(%[fir_coef]) \n\t"
"lwc1 $f0, 120(%[p_data]) \n\t"
"madd.s %[output], %[output], $f2, $f3 \n\t"
"lwc1 $f1, 120(%[fir_coef]) \n\t"
"madd.s %[output], %[output], $f4, $f5 \n\t"
"madd.s %[output], %[output], $f0, $f1 \n\t"

: [output]"=&f"(output)
: [fir_coef]"r"(fir_coef), [p_data]"r"(p_data)
: "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "memory"
);
out[i] = output;
}
memcpy(mem, data + AMRWB_SFR_SIZE_16k, HB_FIR_SIZE * sizeof(float));
}
#endif 
#endif 
