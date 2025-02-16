





















#include <stdlib.h>
#include <string.h>

#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"

#if HAVE_X86ASM

#define cpuid(index, eax, ebx, ecx, edx) ff_cpu_cpuid(index, &eax, &ebx, &ecx, &edx)


#define xgetbv(index, eax, edx) ff_cpu_xgetbv(index, &eax, &edx)


#elif HAVE_INLINE_ASM


#define cpuid(index, eax, ebx, ecx, edx) __asm__ volatile ( "mov %%"FF_REG_b", %%"FF_REG_S" \n\t" "cpuid \n\t" "xchg %%"FF_REG_b", %%"FF_REG_S : "=a" (eax), "=S" (ebx), "=c" (ecx), "=d" (edx) : "0" (index), "2"(0))







#define xgetbv(index, eax, edx) __asm__ (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c" (index))


#define get_eflags(x) __asm__ volatile ("pushfl \n" "pop %0 \n" : "=r"(x))




#define set_eflags(x) __asm__ volatile ("push %0 \n" "popfl \n" :: "r"(x))




#endif 

#if ARCH_X86_64

#define cpuid_test() 1

#elif HAVE_X86ASM

#define cpuid_test ff_cpu_cpuid_test

#elif HAVE_INLINE_ASM

static int cpuid_test(void)
{
x86_reg a, c;



get_eflags(a);
set_eflags(a ^ 0x200000);
get_eflags(c);

return a != c;
}
#endif


int ff_get_cpu_flags_x86(void)
{
int rval = 0;

#if defined(cpuid)

int eax, ebx, ecx, edx;
int max_std_level, max_ext_level, std_caps = 0, ext_caps = 0;
int family = 0, model = 0;
union { int i[3]; char c[12]; } vendor;
int xcr0_lo = 0, xcr0_hi = 0;

if (!cpuid_test())
return 0; 

cpuid(0, max_std_level, vendor.i[0], vendor.i[2], vendor.i[1]);

if (max_std_level >= 1) {
cpuid(1, eax, ebx, ecx, std_caps);
family = ((eax >> 8) & 0xf) + ((eax >> 20) & 0xff);
model = ((eax >> 4) & 0xf) + ((eax >> 12) & 0xf0);
if (std_caps & (1 << 15))
rval |= AV_CPU_FLAG_CMOV;
if (std_caps & (1 << 23))
rval |= AV_CPU_FLAG_MMX;
if (std_caps & (1 << 25))
rval |= AV_CPU_FLAG_MMXEXT;
#if HAVE_SSE
if (std_caps & (1 << 25))
rval |= AV_CPU_FLAG_SSE;
if (std_caps & (1 << 26))
rval |= AV_CPU_FLAG_SSE2;
if (ecx & 1)
rval |= AV_CPU_FLAG_SSE3;
if (ecx & 0x00000200 )
rval |= AV_CPU_FLAG_SSSE3;
if (ecx & 0x00080000 )
rval |= AV_CPU_FLAG_SSE4;
if (ecx & 0x00100000 )
rval |= AV_CPU_FLAG_SSE42;
if (ecx & 0x02000000 )
rval |= AV_CPU_FLAG_AESNI;
#if HAVE_AVX

if ((ecx & 0x18000000) == 0x18000000) {

xgetbv(0, xcr0_lo, xcr0_hi);
if ((xcr0_lo & 0x6) == 0x6) {
rval |= AV_CPU_FLAG_AVX;
if (ecx & 0x00001000)
rval |= AV_CPU_FLAG_FMA3;
}
}
#endif 
#endif 
}
if (max_std_level >= 7) {
cpuid(7, eax, ebx, ecx, edx);
#if HAVE_AVX2
if ((rval & AV_CPU_FLAG_AVX) && (ebx & 0x00000020))
rval |= AV_CPU_FLAG_AVX2;
#if HAVE_AVX512 
if ((xcr0_lo & 0xe0) == 0xe0) { 
if ((rval & AV_CPU_FLAG_AVX2) && (ebx & 0xd0030000) == 0xd0030000)
rval |= AV_CPU_FLAG_AVX512;

}
#endif 
#endif 

if (ebx & 0x00000008) {
rval |= AV_CPU_FLAG_BMI1;
if (ebx & 0x00000100)
rval |= AV_CPU_FLAG_BMI2;
}
}

cpuid(0x80000000, max_ext_level, ebx, ecx, edx);

if (max_ext_level >= 0x80000001) {
cpuid(0x80000001, eax, ebx, ecx, ext_caps);
if (ext_caps & (1U << 31))
rval |= AV_CPU_FLAG_3DNOW;
if (ext_caps & (1 << 30))
rval |= AV_CPU_FLAG_3DNOWEXT;
if (ext_caps & (1 << 23))
rval |= AV_CPU_FLAG_MMX;
if (ext_caps & (1 << 22))
rval |= AV_CPU_FLAG_MMXEXT;

if (!strncmp(vendor.c, "AuthenticAMD", 12)) {







if (rval & AV_CPU_FLAG_SSE2 && !(ecx & 0x00000040))
rval |= AV_CPU_FLAG_SSE2SLOW;







if ((family == 0x15 || family == 0x16) && (rval & AV_CPU_FLAG_AVX))
rval |= AV_CPU_FLAG_AVXSLOW;
}



if (rval & AV_CPU_FLAG_AVX) {
if (ecx & 0x00000800)
rval |= AV_CPU_FLAG_XOP;
if (ecx & 0x00010000)
rval |= AV_CPU_FLAG_FMA4;
}
}

if (!strncmp(vendor.c, "GenuineIntel", 12)) {
if (family == 6 && (model == 9 || model == 13 || model == 14)) {







if (rval & AV_CPU_FLAG_SSE2)
rval ^= AV_CPU_FLAG_SSE2SLOW | AV_CPU_FLAG_SSE2;
if (rval & AV_CPU_FLAG_SSE3)
rval ^= AV_CPU_FLAG_SSE3SLOW | AV_CPU_FLAG_SSE3;
}





if (family == 6 && model == 28)
rval |= AV_CPU_FLAG_ATOM;



if ((rval & AV_CPU_FLAG_SSSE3) && !(rval & AV_CPU_FLAG_SSE4) &&
family == 6 && model < 23)
rval |= AV_CPU_FLAG_SSSE3SLOW;
}

#endif 

return rval;
}

size_t ff_get_cpu_max_align_x86(void)
{
int flags = av_get_cpu_flags();

if (flags & AV_CPU_FLAG_AVX512)
return 64;
if (flags & (AV_CPU_FLAG_AVX2 |
AV_CPU_FLAG_AVX |
AV_CPU_FLAG_XOP |
AV_CPU_FLAG_FMA4 |
AV_CPU_FLAG_FMA3 |
AV_CPU_FLAG_AVXSLOW))
return 32;
if (flags & (AV_CPU_FLAG_AESNI |
AV_CPU_FLAG_SSE42 |
AV_CPU_FLAG_SSE4 |
AV_CPU_FLAG_SSSE3 |
AV_CPU_FLAG_SSE3 |
AV_CPU_FLAG_SSE2 |
AV_CPU_FLAG_SSE |
AV_CPU_FLAG_ATOM |
AV_CPU_FLAG_SSSE3SLOW |
AV_CPU_FLAG_SSE3SLOW |
AV_CPU_FLAG_SSE2SLOW))
return 16;

return 8;
}
