





















































#include "config.h"
#include "libavcodec/acelp_vectors.h"
#include "libavutil/mips/asmdefs.h"

#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
static void ff_weighted_vector_sumf_mips(
float *out, const float *in_a, const float *in_b,
float weight_coeff_a, float weight_coeff_b, int length)
{
const float *a_end = in_a + length;


__asm__ volatile (
"blez %[length], ff_weighted_vector_sumf_end%= \n\t"

"ff_weighted_vector_sumf_madd%=: \n\t"
"lwc1 $f0, 0(%[in_a]) \n\t"
"lwc1 $f3, 4(%[in_a]) \n\t"
"lwc1 $f1, 0(%[in_b]) \n\t"
"lwc1 $f4, 4(%[in_b]) \n\t"
"mul.s $f2, %[weight_coeff_a], $f0 \n\t"
"mul.s $f5, %[weight_coeff_a], $f3 \n\t"
"madd.s $f2, $f2, %[weight_coeff_b], $f1 \n\t"
"madd.s $f5, $f5, %[weight_coeff_b], $f4 \n\t"
PTR_ADDIU "%[in_a],8 \n\t"
PTR_ADDIU "%[in_b],8 \n\t"
"swc1 $f2, 0(%[out]) \n\t"
"swc1 $f5, 4(%[out]) \n\t"
PTR_ADDIU "%[out], 8 \n\t"
"bne %[in_a], %[a_end], ff_weighted_vector_sumf_madd%= \n\t"

"ff_weighted_vector_sumf_end%=: \n\t"

: [out] "+r" (out), [in_a] "+r" (in_a), [in_b] "+r" (in_b)
: [weight_coeff_a] "f" (weight_coeff_a),
[weight_coeff_b] "f" (weight_coeff_b),
[length] "r" (length), [a_end]"r"(a_end)
: "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "memory"
);
}
#endif 
#endif 

void ff_acelp_vectors_init_mips(ACELPVContext *c)
{
#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
c->weighted_vector_sumf = ff_weighted_vector_sumf_mips;
#endif
#endif
}
