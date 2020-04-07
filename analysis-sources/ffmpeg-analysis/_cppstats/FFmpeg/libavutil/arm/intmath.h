#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"
#if HAVE_INLINE_ASM
#if HAVE_ARMV6_INLINE
#define av_clip_uint8 av_clip_uint8_arm
static av_always_inline av_const int av_clip_uint8_arm(int a)
{
int x;
__asm__ ("usat %0, #8, %1" : "=r"(x) : "r"(a));
return x;
}
#define av_clip_int8 av_clip_int8_arm
static av_always_inline av_const int av_clip_int8_arm(int a)
{
int x;
__asm__ ("ssat %0, #8, %1" : "=r"(x) : "r"(a));
return x;
}
#define av_clip_uint16 av_clip_uint16_arm
static av_always_inline av_const int av_clip_uint16_arm(int a)
{
int x;
__asm__ ("usat %0, #16, %1" : "=r"(x) : "r"(a));
return x;
}
#define av_clip_int16 av_clip_int16_arm
static av_always_inline av_const int av_clip_int16_arm(int a)
{
int x;
__asm__ ("ssat %0, #16, %1" : "=r"(x) : "r"(a));
return x;
}
#define av_clip_intp2 av_clip_intp2_arm
static av_always_inline av_const int av_clip_intp2_arm(int a, int p)
{
unsigned x;
__asm__ ("ssat %0, %2, %1" : "=r"(x) : "r"(a), "i"(p+1));
return x;
}
#define av_clip_uintp2 av_clip_uintp2_arm
static av_always_inline av_const unsigned av_clip_uintp2_arm(int a, int p)
{
unsigned x;
__asm__ ("usat %0, %2, %1" : "=r"(x) : "r"(a), "i"(p));
return x;
}
#define av_sat_add32 av_sat_add32_arm
static av_always_inline int av_sat_add32_arm(int a, int b)
{
int r;
__asm__ ("qadd %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
return r;
}
#define av_sat_dadd32 av_sat_dadd32_arm
static av_always_inline int av_sat_dadd32_arm(int a, int b)
{
int r;
__asm__ ("qdadd %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
return r;
}
#define av_sat_sub32 av_sat_sub32_arm
static av_always_inline int av_sat_sub32_arm(int a, int b)
{
int r;
__asm__ ("qsub %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
return r;
}
#define av_sat_dsub32 av_sat_dsub32_arm
static av_always_inline int av_sat_dsub32_arm(int a, int b)
{
int r;
__asm__ ("qdsub %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
return r;
}
#endif 
#if HAVE_ASM_MOD_Q
#define av_clipl_int32 av_clipl_int32_arm
static av_always_inline av_const int32_t av_clipl_int32_arm(int64_t a)
{
int x, y;
__asm__ ("adds %1, %R2, %Q2, lsr #31 \n\t"
"itet ne \n\t"
"mvnne %1, #1<<31 \n\t"
"moveq %0, %Q2 \n\t"
"eorne %0, %1, %R2, asr #31 \n\t"
: "=r"(x), "=&r"(y) : "r"(a) : "cc");
return x;
}
#endif 
#endif 
