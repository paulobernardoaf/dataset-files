#include <stdint.h>
#include "config.h"
#if HAVE_XFORM_ASM
#if HAVE_BIGENDIAN
#define AV_RL16 av_read_bswap16
#define AV_WL16 av_write_bswap16
#define AV_RL32 av_read_bswap32
#define AV_WL32 av_write_bswap32
#define AV_RL64 av_read_bswap64
#define AV_WL64 av_write_bswap64
#else
#define AV_RB16 av_read_bswap16
#define AV_WB16 av_write_bswap16
#define AV_RB32 av_read_bswap32
#define AV_WB32 av_write_bswap32
#define AV_RB64 av_read_bswap64
#define AV_WB64 av_write_bswap64
#endif
static av_always_inline uint16_t av_read_bswap16(const void *p)
{
uint16_t v;
__asm__ ("lhbrx %0, %y1" : "=r"(v) : "Z"(*(const uint16_t*)p));
return v;
}
static av_always_inline void av_write_bswap16(void *p, uint16_t v)
{
__asm__ ("sthbrx %1, %y0" : "=Z"(*(uint16_t*)p) : "r"(v));
}
static av_always_inline uint32_t av_read_bswap32(const void *p)
{
uint32_t v;
__asm__ ("lwbrx %0, %y1" : "=r"(v) : "Z"(*(const uint32_t*)p));
return v;
}
static av_always_inline void av_write_bswap32(void *p, uint32_t v)
{
__asm__ ("stwbrx %1, %y0" : "=Z"(*(uint32_t*)p) : "r"(v));
}
#if HAVE_LDBRX
static av_always_inline uint64_t av_read_bswap64(const void *p)
{
uint64_t v;
__asm__ ("ldbrx %0, %y1" : "=r"(v) : "Z"(*(const uint64_t*)p));
return v;
}
static av_always_inline void av_write_bswap64(void *p, uint64_t v)
{
__asm__ ("stdbrx %1, %y0" : "=Z"(*(uint64_t*)p) : "r"(v));
}
#else
static av_always_inline uint64_t av_read_bswap64(const void *p)
{
union { uint64_t v; uint32_t hl[2]; } v;
__asm__ ("lwbrx %0, %y2 \n\t"
"lwbrx %1, %y3 \n\t"
: "=&r"(v.hl[1]), "=r"(v.hl[0])
: "Z"(*(const uint32_t*)p), "Z"(*((const uint32_t*)p+1)));
return v.v;
}
static av_always_inline void av_write_bswap64(void *p, uint64_t v)
{
union { uint64_t v; uint32_t hl[2]; } vv = { v };
__asm__ ("stwbrx %2, %y0 \n\t"
"stwbrx %3, %y1 \n\t"
: "=Z"(*(uint32_t*)p), "=Z"(*((uint32_t*)p+1))
: "r"(vv.hl[1]), "r"(vv.hl[0]));
}
#endif 
#endif 
