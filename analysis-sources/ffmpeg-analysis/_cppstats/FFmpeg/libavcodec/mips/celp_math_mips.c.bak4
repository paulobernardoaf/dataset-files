




















































#include "config.h"
#include "libavcodec/celp_math.h"
#include "libavutil/mips/asmdefs.h"

#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
static float ff_dot_productf_mips(const float* a, const float* b,
int length)
{
float sum;
const float* a_end = a + length;

__asm__ volatile (
"mtc1 $zero, %[sum] \n\t"
"blez %[length], ff_dot_productf_end%= \n\t"
"ff_dot_productf_madd%=: \n\t"
"lwc1 $f2, 0(%[a]) \n\t"
"lwc1 $f1, 0(%[b]) \n\t"
PTR_ADDIU "%[a], %[a], 4 \n\t"
PTR_ADDIU "%[b], %[b], 4 \n\t"
"madd.s %[sum], %[sum], $f1, $f2 \n\t"
"bne %[a], %[a_end], ff_dot_productf_madd%= \n\t"
"ff_dot_productf_end%=: \n\t"

: [sum] "=&f" (sum), [a] "+r" (a), [b] "+r" (b)
: [a_end]"r"(a_end), [length] "r" (length)
: "$f1", "$f2", "memory"
);
return sum;
}
#endif 
#endif 

void ff_celp_math_init_mips(CELPMContext *c)
{
#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
c->dot_productf = ff_dot_productf_mips;
#endif
#endif
}
