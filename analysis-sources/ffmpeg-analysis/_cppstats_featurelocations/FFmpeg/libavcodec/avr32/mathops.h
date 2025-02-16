




















#if !defined(AVCODEC_AVR32_MATHOPS_H)
#define AVCODEC_AVR32_MATHOPS_H

#include <stdint.h>
#include "config.h"
#include "libavutil/common.h"

#if HAVE_INLINE_ASM

#define MULL MULL
static inline av_const int MULL(int a, int b, unsigned shift)
{
union { int64_t x; int hl[2]; } x;
__asm__ ("muls.d %0, %1, %2 \n\t"
"lsr %0, %3 \n\t"
"or %0, %0, %m0<<%4 \n\t"
: "=r"(x) : "r"(b), "r"(a), "i"(shift), "i"(32-shift));
return x.hl[1];
}

#define MULH MULH
static inline av_const int MULH(int a, int b)
{
union { int64_t x; int hl[2]; } x;
__asm__ ("muls.d %0, %1, %2" : "=r"(x.x) : "r"(a), "r"(b));
return x.hl[0];
}

#define MUL64 MUL64
static inline av_const int64_t MUL64(int a, int b)
{
int64_t x;
__asm__ ("muls.d %0, %1, %2" : "=r"(x) : "r"(a), "r"(b));
return x;
}

static inline av_const int64_t MAC64(int64_t d, int a, int b)
{
__asm__ ("macs.d %0, %1, %2" : "+r"(d) : "r"(a), "r"(b));
return d;
}
#define MAC64(d, a, b) ((d) = MAC64(d, a, b))
#define MLS64(d, a, b) MAC64(d, -(a), b)

static inline av_const int MAC16(int d, int a, int b)
{
__asm__ ("machh.w %0, %1:b, %2:b" : "+r"(d) : "r"(a), "r"(b));
return d;
}
#define MAC16(d, a, b) ((d) = MAC16(d, a, b))
#define MLS16(d, a, b) MAC16(d, -(a), b)

#define MUL16 MUL16
static inline av_const int MUL16(int a, int b)
{
int d;
__asm__ ("mulhh.w %0, %1:b, %2:b" : "=r"(d) : "r"(a), "r"(b));
return d;
}

#define mid_pred mid_pred
static inline av_const int mid_pred(int a, int b, int c)
{
int m;
__asm__ ("mov %0, %2 \n\t"
"cp.w %1, %2 \n\t"
"movgt %0, %1 \n\t"
"movgt %1, %2 \n\t"
"cp.w %1, %3 \n\t"
"movle %1, %3 \n\t"
"cp.w %0, %1 \n\t"
"movgt %0, %1 \n\t"
: "=&r"(m), "+r"(a)
: "r"(b), "r"(c));
return m;
}

#endif 

#endif 
