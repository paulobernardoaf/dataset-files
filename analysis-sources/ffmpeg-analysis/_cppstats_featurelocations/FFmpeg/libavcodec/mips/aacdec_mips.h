























































#if !defined(AVCODEC_MIPS_AACDEC_MIPS_H)
#define AVCODEC_MIPS_AACDEC_MIPS_H

#include "libavcodec/aac.h"
#include "libavutil/mips/asmdefs.h"

#if HAVE_INLINE_ASM && HAVE_MIPSFPU
#if !HAVE_MIPS32R6 && !HAVE_MIPS64R6
static inline float *VMUL2_mips(float *dst, const float *v, unsigned idx,
const float *scale)
{
float temp0, temp1, temp2;
int temp3, temp4;
float *ret;

__asm__ volatile(
"andi %[temp3], %[idx], 0x0F \n\t"
"andi %[temp4], %[idx], 0xF0 \n\t"
"sll %[temp3], %[temp3], 2 \n\t"
"srl %[temp4], %[temp4], 2 \n\t"
"lwc1 %[temp2], 0(%[scale]) \n\t"
"lwxc1 %[temp0], %[temp3](%[v]) \n\t"
"lwxc1 %[temp1], %[temp4](%[v]) \n\t"
"mul.s %[temp0], %[temp0], %[temp2] \n\t"
"mul.s %[temp1], %[temp1], %[temp2] \n\t"
PTR_ADDIU "%[ret], %[dst], 8 \n\t"
"swc1 %[temp0], 0(%[dst]) \n\t"
"swc1 %[temp1], 4(%[dst]) \n\t"

: [temp0]"=&f"(temp0), [temp1]"=&f"(temp1),
[temp2]"=&f"(temp2), [temp3]"=&r"(temp3),
[temp4]"=&r"(temp4), [ret]"=&r"(ret)
: [idx]"r"(idx), [scale]"r"(scale), [v]"r"(v),
[dst]"r"(dst)
: "memory"
);
return ret;
}

static inline float *VMUL4_mips(float *dst, const float *v, unsigned idx,
const float *scale)
{
int temp0, temp1, temp2, temp3;
float temp4, temp5, temp6, temp7, temp8;
float *ret;

__asm__ volatile(
"andi %[temp0], %[idx], 0x03 \n\t"
"andi %[temp1], %[idx], 0x0C \n\t"
"andi %[temp2], %[idx], 0x30 \n\t"
"andi %[temp3], %[idx], 0xC0 \n\t"
"sll %[temp0], %[temp0], 2 \n\t"
"srl %[temp2], %[temp2], 2 \n\t"
"srl %[temp3], %[temp3], 4 \n\t"
"lwc1 %[temp4], 0(%[scale]) \n\t"
"lwxc1 %[temp5], %[temp0](%[v]) \n\t"
"lwxc1 %[temp6], %[temp1](%[v]) \n\t"
"lwxc1 %[temp7], %[temp2](%[v]) \n\t"
"lwxc1 %[temp8], %[temp3](%[v]) \n\t"
"mul.s %[temp5], %[temp5], %[temp4] \n\t"
"mul.s %[temp6], %[temp6], %[temp4] \n\t"
"mul.s %[temp7], %[temp7], %[temp4] \n\t"
"mul.s %[temp8], %[temp8], %[temp4] \n\t"
PTR_ADDIU "%[ret], %[dst], 16 \n\t"
"swc1 %[temp5], 0(%[dst]) \n\t"
"swc1 %[temp6], 4(%[dst]) \n\t"
"swc1 %[temp7], 8(%[dst]) \n\t"
"swc1 %[temp8], 12(%[dst]) \n\t"

: [temp0]"=&r"(temp0), [temp1]"=&r"(temp1),
[temp2]"=&r"(temp2), [temp3]"=&r"(temp3),
[temp4]"=&f"(temp4), [temp5]"=&f"(temp5),
[temp6]"=&f"(temp6), [temp7]"=&f"(temp7),
[temp8]"=&f"(temp8), [ret]"=&r"(ret)
: [idx]"r"(idx), [scale]"r"(scale), [v]"r"(v),
[dst]"r"(dst)
: "memory"
);
return ret;
}

static inline float *VMUL2S_mips(float *dst, const float *v, unsigned idx,
unsigned sign, const float *scale)
{
int temp0, temp1, temp2, temp3, temp4, temp5;
float temp6, temp7, temp8, temp9;
float *ret;

__asm__ volatile(
"andi %[temp0], %[idx], 0x0F \n\t"
"andi %[temp1], %[idx], 0xF0 \n\t"
"lw %[temp4], 0(%[scale]) \n\t"
"srl %[temp2], %[sign], 1 \n\t"
"sll %[temp3], %[sign], 31 \n\t"
"sll %[temp2], %[temp2], 31 \n\t"
"sll %[temp0], %[temp0], 2 \n\t"
"srl %[temp1], %[temp1], 2 \n\t"
"lwxc1 %[temp8], %[temp0](%[v]) \n\t"
"lwxc1 %[temp9], %[temp1](%[v]) \n\t"
"xor %[temp5], %[temp4], %[temp2] \n\t"
"xor %[temp4], %[temp4], %[temp3] \n\t"
"mtc1 %[temp5], %[temp6] \n\t"
"mtc1 %[temp4], %[temp7] \n\t"
"mul.s %[temp8], %[temp8], %[temp6] \n\t"
"mul.s %[temp9], %[temp9], %[temp7] \n\t"
PTR_ADDIU "%[ret], %[dst], 8 \n\t"
"swc1 %[temp8], 0(%[dst]) \n\t"
"swc1 %[temp9], 4(%[dst]) \n\t"

: [temp0]"=&r"(temp0), [temp1]"=&r"(temp1),
[temp2]"=&r"(temp2), [temp3]"=&r"(temp3),
[temp4]"=&r"(temp4), [temp5]"=&r"(temp5),
[temp6]"=&f"(temp6), [temp7]"=&f"(temp7),
[temp8]"=&f"(temp8), [temp9]"=&f"(temp9),
[ret]"=&r"(ret)
: [idx]"r"(idx), [scale]"r"(scale), [v]"r"(v),
[dst]"r"(dst), [sign]"r"(sign)
: "memory"
);
return ret;
}

static inline float *VMUL4S_mips(float *dst, const float *v, unsigned idx,
unsigned sign, const float *scale)
{
int temp0, temp1, temp2, temp3, temp4;
float temp10, temp11, temp12, temp13, temp14, temp15, temp16, temp17;
float *ret;
unsigned int mask = 1U << 31;

__asm__ volatile(
"lw %[temp0], 0(%[scale]) \n\t"
"andi %[temp1], %[idx], 0x03 \n\t"
"andi %[temp2], %[idx], 0x0C \n\t"
"andi %[temp3], %[idx], 0x30 \n\t"
"andi %[temp4], %[idx], 0xC0 \n\t"
"sll %[temp1], %[temp1], 2 \n\t"
"srl %[temp3], %[temp3], 2 \n\t"
"srl %[temp4], %[temp4], 4 \n\t"
"lwxc1 %[temp10], %[temp1](%[v]) \n\t"
"lwxc1 %[temp11], %[temp2](%[v]) \n\t"
"lwxc1 %[temp12], %[temp3](%[v]) \n\t"
"lwxc1 %[temp13], %[temp4](%[v]) \n\t"
"and %[temp1], %[sign], %[mask] \n\t"
"srl %[temp2], %[idx], 12 \n\t"
"srl %[temp3], %[idx], 13 \n\t"
"srl %[temp4], %[idx], 14 \n\t"
"andi %[temp2], %[temp2], 1 \n\t"
"andi %[temp3], %[temp3], 1 \n\t"
"andi %[temp4], %[temp4], 1 \n\t"
"sllv %[sign], %[sign], %[temp2] \n\t"
"xor %[temp1], %[temp0], %[temp1] \n\t"
"and %[temp2], %[sign], %[mask] \n\t"
"mtc1 %[temp1], %[temp14] \n\t"
"xor %[temp2], %[temp0], %[temp2] \n\t"
"sllv %[sign], %[sign], %[temp3] \n\t"
"mtc1 %[temp2], %[temp15] \n\t"
"and %[temp3], %[sign], %[mask] \n\t"
"sllv %[sign], %[sign], %[temp4] \n\t"
"xor %[temp3], %[temp0], %[temp3] \n\t"
"and %[temp4], %[sign], %[mask] \n\t"
"mtc1 %[temp3], %[temp16] \n\t"
"xor %[temp4], %[temp0], %[temp4] \n\t"
"mtc1 %[temp4], %[temp17] \n\t"
"mul.s %[temp10], %[temp10], %[temp14] \n\t"
"mul.s %[temp11], %[temp11], %[temp15] \n\t"
"mul.s %[temp12], %[temp12], %[temp16] \n\t"
"mul.s %[temp13], %[temp13], %[temp17] \n\t"
PTR_ADDIU "%[ret], %[dst], 16 \n\t"
"swc1 %[temp10], 0(%[dst]) \n\t"
"swc1 %[temp11], 4(%[dst]) \n\t"
"swc1 %[temp12], 8(%[dst]) \n\t"
"swc1 %[temp13], 12(%[dst]) \n\t"

: [temp0]"=&r"(temp0), [temp1]"=&r"(temp1),
[temp2]"=&r"(temp2), [temp3]"=&r"(temp3),
[temp4]"=&r"(temp4), [temp10]"=&f"(temp10),
[temp11]"=&f"(temp11), [temp12]"=&f"(temp12),
[temp13]"=&f"(temp13), [temp14]"=&f"(temp14),
[temp15]"=&f"(temp15), [temp16]"=&f"(temp16),
[temp17]"=&f"(temp17), [ret]"=&r"(ret),
[sign]"+r"(sign)
: [idx]"r"(idx), [scale]"r"(scale), [v]"r"(v),
[dst]"r"(dst), [mask]"r"(mask)
: "memory"
);
return ret;
}

#define VMUL2 VMUL2_mips
#define VMUL4 VMUL4_mips
#define VMUL2S VMUL2S_mips
#define VMUL4S VMUL4S_mips
#endif 
#endif 

#endif 
