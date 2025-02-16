






















#if !defined(AVUTIL_X86_BSWAP_H)
#define AVUTIL_X86_BSWAP_H

#include <stdint.h>
#if defined(_MSC_VER)
#include <stdlib.h>
#include <intrin.h>
#endif
#include "config.h"
#include "libavutil/attributes.h"

#if defined(_MSC_VER)

#define av_bswap16 av_bswap16
static av_always_inline av_const uint16_t av_bswap16(uint16_t x)
{
return _rotr16(x, 8);
}

#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
return _byteswap_ulong(x);
}

#if ARCH_X86_64
#define av_bswap64 av_bswap64
static inline uint64_t av_const av_bswap64(uint64_t x)
{
return _byteswap_uint64(x);
}
#endif


#elif HAVE_INLINE_ASM

#if AV_GCC_VERSION_AT_MOST(4,0)
#define av_bswap16 av_bswap16
static av_always_inline av_const unsigned av_bswap16(unsigned x)
{
__asm__("rorw $8, %w0" : "+r"(x));
return x;
}
#endif 

#if AV_GCC_VERSION_AT_MOST(4,4) || defined(__INTEL_COMPILER)
#define av_bswap32 av_bswap32
static av_always_inline av_const uint32_t av_bswap32(uint32_t x)
{
__asm__("bswap %0" : "+r" (x));
return x;
}

#if ARCH_X86_64
#define av_bswap64 av_bswap64
static inline uint64_t av_const av_bswap64(uint64_t x)
{
__asm__("bswap %0": "=r" (x) : "0" (x));
return x;
}
#endif
#endif 

#endif 
#endif 
