#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"
#define AV_RB16 AV_RB16
static av_always_inline uint16_t AV_RB16(const void *p)
{
uint16_t v;
__asm__ ("loadacc, (%1+) \n\t"
"rol8 \n\t"
"storeacc, %0 \n\t"
"loadacc, (%1+) \n\t"
"add, %0 \n\t"
: "=r"(v), "+a"(p));
return v;
}
#define AV_WB16 AV_WB16
static av_always_inline void AV_WB16(void *p, uint16_t v)
{
__asm__ volatile ("loadacc, %1 \n\t"
"lsr8 \n\t"
"storeacc, (%0+) \n\t"
"loadacc, %1 \n\t"
"storeacc, (%0+) \n\t"
: "+&a"(p) : "r"(v));
}
#define AV_RL16 AV_RL16
static av_always_inline uint16_t AV_RL16(const void *p)
{
uint16_t v;
__asm__ ("loadacc, (%1+) \n\t"
"storeacc, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"add, %0 \n\t"
: "=r"(v), "+a"(p));
return v;
}
#define AV_WL16 AV_WL16
static av_always_inline void AV_WL16(void *p, uint16_t v)
{
__asm__ volatile ("loadacc, %1 \n\t"
"storeacc, (%0+) \n\t"
"lsr8 \n\t"
"storeacc, (%0+) \n\t"
: "+&a"(p) : "r"(v));
}
#define AV_RB32 AV_RB32
static av_always_inline uint32_t AV_RB32(const void *p)
{
uint32_t v;
__asm__ ("loadacc, (%1+) \n\t"
"rol8 \n\t"
"rol8 \n\t"
"rol8 \n\t"
"storeacc, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"rol8 \n\t"
"add, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"add, %0 \n\t"
"loadacc, (%1+) \n\t"
"add, %0 \n\t"
: "=r"(v), "+a"(p));
return v;
}
#define AV_WB32 AV_WB32
static av_always_inline void AV_WB32(void *p, uint32_t v)
{
__asm__ volatile ("loadacc, #4 \n\t"
"add, %0 \n\t"
"loadacc, %1 \n\t"
"storeacc, (-%0) \n\t"
"lsr8 \n\t"
"storeacc, (-%0) \n\t"
"lsr8 \n\t"
"storeacc, (-%0) \n\t"
"lsr8 \n\t"
"storeacc, (-%0) \n\t"
: "+&a"(p) : "r"(v));
}
#define AV_RL32 AV_RL32
static av_always_inline uint32_t AV_RL32(const void *p)
{
uint32_t v;
__asm__ ("loadacc, (%1+) \n\t"
"storeacc, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"add, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"rol8 \n\t"
"add, %0 \n\t"
"loadacc, (%1+) \n\t"
"rol8 \n\t"
"rol8 \n\t"
"rol8 \n\t"
"add, %0 \n\t"
: "=r"(v), "+a"(p));
return v;
}
#define AV_WL32 AV_WL32
static av_always_inline void AV_WL32(void *p, uint32_t v)
{
__asm__ volatile ("loadacc, %1 \n\t"
"storeacc, (%0+) \n\t"
"lsr8 \n\t"
"storeacc, (%0+) \n\t"
"lsr8 \n\t"
"storeacc, (%0+) \n\t"
"lsr8 \n\t"
"storeacc, (%0+) \n\t"
: "+&a"(p) : "r"(v));
}
