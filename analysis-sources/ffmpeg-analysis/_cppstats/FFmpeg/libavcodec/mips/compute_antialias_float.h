#include "libavutil/mips/asmdefs.h"
#if HAVE_INLINE_ASM
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
static void compute_antialias_mips_float(MPADecodeContext *s,
GranuleDef *g)
{
float *ptr, *ptr_end;
float *csa = &csa_table[0][0];
float in1, in2, in3, in4, in5, in6, in7, in8;
float out1, out2, out3, out4;
ptr = g->sb_hybrid + 18;
if (g->block_type == 2) {
if (!g->switch_point)
return;
ptr_end = ptr + 18;
} else {
ptr_end = ptr + 558;
}
__asm__ volatile (
"compute_antialias_float_loop%=: \t\n"
"lwc1 %[in1], -1*4(%[ptr]) \t\n"
"lwc1 %[in2], 0(%[csa]) \t\n"
"lwc1 %[in3], 1*4(%[csa]) \t\n"
"lwc1 %[in4], 0(%[ptr]) \t\n"
"lwc1 %[in5], -2*4(%[ptr]) \t\n"
"lwc1 %[in6], 4*4(%[csa]) \t\n"
"mul.s %[out1], %[in1], %[in2] \t\n"
"mul.s %[out2], %[in1], %[in3] \t\n"
"lwc1 %[in7], 5*4(%[csa]) \t\n"
"lwc1 %[in8], 1*4(%[ptr]) \t\n"
"nmsub.s %[out1], %[out1], %[in3], %[in4] \t\n"
"madd.s %[out2], %[out2], %[in2], %[in4] \t\n"
"mul.s %[out3], %[in5], %[in6] \t\n"
"mul.s %[out4], %[in5], %[in7] \t\n"
"lwc1 %[in1], -3*4(%[ptr]) \t\n"
"swc1 %[out1], -1*4(%[ptr]) \t\n"
"swc1 %[out2], 0(%[ptr]) \t\n"
"nmsub.s %[out3], %[out3], %[in7], %[in8] \t\n"
"madd.s %[out4], %[out4], %[in6], %[in8] \t\n"
"lwc1 %[in2], 8*4(%[csa]) \t\n"
"swc1 %[out3], -2*4(%[ptr]) \t\n"
"swc1 %[out4], 1*4(%[ptr]) \t\n"
"lwc1 %[in3], 9*4(%[csa]) \t\n"
"lwc1 %[in4], 2*4(%[ptr]) \t\n"
"mul.s %[out1], %[in1], %[in2] \t\n"
"lwc1 %[in5], -4*4(%[ptr]) \t\n"
"lwc1 %[in6], 12*4(%[csa]) \t\n"
"mul.s %[out2], %[in1], %[in3] \t\n"
"lwc1 %[in7], 13*4(%[csa]) \t\n"
"nmsub.s %[out1], %[out1], %[in3], %[in4] \t\n"
"lwc1 %[in8], 3*4(%[ptr]) \t\n"
"mul.s %[out3], %[in5], %[in6] \t\n"
"madd.s %[out2], %[out2], %[in2], %[in4] \t\n"
"mul.s %[out4], %[in5], %[in7] \t\n"
"swc1 %[out1], -3*4(%[ptr]) \t\n"
"lwc1 %[in1], -5*4(%[ptr]) \t\n"
"nmsub.s %[out3], %[out3], %[in7], %[in8] \t\n"
"swc1 %[out2], 2*4(%[ptr]) \t\n"
"madd.s %[out4], %[out4], %[in6], %[in8] \t\n"
"lwc1 %[in2], 16*4(%[csa]) \t\n"
"lwc1 %[in3], 17*4(%[csa]) \t\n"
"swc1 %[out3], -4*4(%[ptr]) \t\n"
"lwc1 %[in4], 4*4(%[ptr]) \t\n"
"swc1 %[out4], 3*4(%[ptr]) \t\n"
"mul.s %[out1], %[in1], %[in2] \t\n"
"mul.s %[out2], %[in1], %[in3] \t\n"
"lwc1 %[in5], -6*4(%[ptr]) \t\n"
"lwc1 %[in6], 20*4(%[csa]) \t\n"
"lwc1 %[in7], 21*4(%[csa]) \t\n"
"nmsub.s %[out1], %[out1], %[in3], %[in4] \t\n"
"madd.s %[out2], %[out2], %[in2], %[in4] \t\n"
"lwc1 %[in8], 5*4(%[ptr]) \t\n"
"mul.s %[out3], %[in5], %[in6] \t\n"
"mul.s %[out4], %[in5], %[in7] \t\n"
"swc1 %[out1], -5*4(%[ptr]) \t\n"
"swc1 %[out2], 4*4(%[ptr]) \t\n"
"lwc1 %[in1], -7*4(%[ptr]) \t\n"
"nmsub.s %[out3], %[out3], %[in7], %[in8] \t\n"
"madd.s %[out4], %[out4], %[in6], %[in8] \t\n"
"lwc1 %[in2], 24*4(%[csa]) \t\n"
"lwc1 %[in3], 25*4(%[csa]) \t\n"
"lwc1 %[in4], 6*4(%[ptr]) \t\n"
"swc1 %[out3], -6*4(%[ptr]) \t\n"
"swc1 %[out4], 5*4(%[ptr]) \t\n"
"mul.s %[out1], %[in1], %[in2] \t\n"
"lwc1 %[in5], -8*4(%[ptr]) \t\n"
"mul.s %[out2], %[in1], %[in3] \t\n"
"lwc1 %[in6], 28*4(%[csa]) \t\n"
"lwc1 %[in7], 29*4(%[csa]) \t\n"
"nmsub.s %[out1], %[out1], %[in3], %[in4] \t\n"
"lwc1 %[in8], 7*4(%[ptr]) \t\n"
"madd.s %[out2], %[out2], %[in2], %[in4] \t\n"
"mul.s %[out3], %[in5], %[in6] \t\n"
"mul.s %[out4], %[in5], %[in7] \t\n"
"swc1 %[out1], -7*4(%[ptr]) \t\n"
"swc1 %[out2], 6*4(%[ptr]) \t\n"
PTR_ADDIU "%[ptr],%[ptr], 72 \t\n"
"nmsub.s %[out3], %[out3], %[in7], %[in8] \t\n"
"madd.s %[out4], %[out4], %[in6], %[in8] \t\n"
"swc1 %[out3], -26*4(%[ptr]) \t\n"
"swc1 %[out4], -11*4(%[ptr]) \t\n"
"bne %[ptr], %[ptr_end], compute_antialias_float_loop%= \t\n"
: [ptr] "+r" (ptr),
[in1] "=&f" (in1), [in2] "=&f" (in2),
[in3] "=&f" (in3), [in4] "=&f" (in4),
[in5] "=&f" (in5), [in6] "=&f" (in6),
[in7] "=&f" (in7), [in8] "=&f" (in8),
[out1] "=&f" (out1), [out2] "=&f" (out2),
[out3] "=&f" (out3), [out4] "=&f" (out4)
: [csa] "r" (csa), [ptr_end] "r" (ptr_end)
: "memory"
);
}
#define compute_antialias compute_antialias_mips_float
#endif 
#endif 
