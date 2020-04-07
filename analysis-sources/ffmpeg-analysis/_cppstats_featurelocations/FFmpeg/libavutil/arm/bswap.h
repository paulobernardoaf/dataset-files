

















#if !defined(AVUTIL_ARM_BSWAP_H)
#define AVUTIL_ARM_BSWAP_H

#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"

#if defined(__ARMCC_VERSION)

#if HAVE_ARMV6
#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
return __rev(x);
}
#endif 

#elif HAVE_INLINE_ASM

#if HAVE_ARMV6_INLINE
#define av_bswap16 av_bswap16
static av_always_inline av_const unsigned av_bswap16(unsigned x)
{
__asm__("rev16 %0, %0" : "+r"(x));
return x;
}
#endif

#if AV_GCC_VERSION_AT_MOST(4,4)
#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
#if HAVE_ARMV6_INLINE
__asm__("rev %0, %0" : "+r"(x));
#else
uint32_t t;
__asm__ ("eor %1, %0, %0, ror #16 \n\t"
"bic %1, %1, #0xFF0000 \n\t"
"mov %0, %0, ror #8 \n\t"
"eor %0, %0, %1, lsr #8 \n\t"
: "+r"(x), "=&r"(t));
#endif 
return x;
}
#endif 

#endif 

#endif 
