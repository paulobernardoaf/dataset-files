






















#if !defined(AVUTIL_SH4_BSWAP_H)
#define AVUTIL_SH4_BSWAP_H

#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"

#define av_bswap16 av_bswap16
static av_always_inline av_const uint16_t av_bswap16(uint16_t x)
{
__asm__("swap.b %0,%0" : "+r"(x));
return x;
}

#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
__asm__("swap.b %0,%0\n"
"swap.w %0,%0\n"
"swap.b %0,%0\n"
: "+r"(x));
return x;
}

#endif 
