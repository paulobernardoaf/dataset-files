#include <stdint.h>
#include "config.h"
#define AV_READ_TIME read_time
static inline uint64_t read_time(void)
{
uint32_t tbu, tbl, temp;
__asm__ volatile(
"mftbu %2\n"
"mftb %0\n"
"mftbu %1\n"
"cmpw %2,%1\n"
"bne $-0x10\n"
: "=r"(tbl), "=r"(tbu), "=r"(temp)
:
: "cc");
return (((uint64_t)tbu)<<32) | (uint64_t)tbl;
}
