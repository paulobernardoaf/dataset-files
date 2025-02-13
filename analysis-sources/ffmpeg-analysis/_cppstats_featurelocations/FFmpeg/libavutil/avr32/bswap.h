

















#if !defined(AVUTIL_AVR32_BSWAP_H)
#define AVUTIL_AVR32_BSWAP_H

#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"

#if HAVE_INLINE_ASM

#define av_bswap16 av_bswap16
static av_always_inline av_const uint16_t av_bswap16(uint16_t x)
{
__asm__ ("swap.bh %0" : "+r"(x));
return x;
}

#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
__asm__ ("swap.b %0" : "+r"(x));
return x;
}

#endif 

#endif 
