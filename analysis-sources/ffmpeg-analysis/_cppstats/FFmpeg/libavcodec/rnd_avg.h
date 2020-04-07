#include <stddef.h>
#include <stdint.h>
#define BYTE_VEC32(c) ((c) * 0x01010101UL)
#define BYTE_VEC64(c) ((c) * 0x0001000100010001UL)
static inline uint32_t rnd_avg32(uint32_t a, uint32_t b)
{
return (a | b) - (((a ^ b) & ~BYTE_VEC32(0x01)) >> 1);
}
static inline uint32_t no_rnd_avg32(uint32_t a, uint32_t b)
{
return (a & b) + (((a ^ b) & ~BYTE_VEC32(0x01)) >> 1);
}
static inline uint64_t rnd_avg64(uint64_t a, uint64_t b)
{
return (a | b) - (((a ^ b) & ~BYTE_VEC64(0x01)) >> 1);
}
static inline uint64_t no_rnd_avg64(uint64_t a, uint64_t b)
{
return (a & b) + (((a ^ b) & ~BYTE_VEC64(0x01)) >> 1);
}
