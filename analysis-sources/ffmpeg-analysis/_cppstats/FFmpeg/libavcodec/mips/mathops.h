#include <stdint.h>
#include "config.h"
#include "libavutil/common.h"
#if HAVE_INLINE_ASM
#if HAVE_LOONGSON3
#define MULH MULH
static inline av_const int MULH(int a, int b)
{
int c;
__asm__ ("dmult %1, %2 \n\t"
"mflo %0 \n\t"
"dsrl %0, %0, 32 \n\t"
: "=r"(c)
: "r"(a),"r"(b)
: "hi", "lo");
return c;
}
#define mid_pred mid_pred
static inline av_const int mid_pred(int a, int b, int c)
{
int t = b;
__asm__ ("sgt $8, %1, %2 \n\t"
"movn %0, %1, $8 \n\t"
"movn %1, %2, $8 \n\t"
"sgt $8, %1, %3 \n\t"
"movz %1, %3, $8 \n\t"
"sgt $8, %0, %1 \n\t"
"movn %0, %1, $8 \n\t"
: "+&r"(t),"+&r"(a)
: "r"(b),"r"(c)
: "$8");
return t;
}
#endif 
#endif 
