#include <stdint.h>
#define AV_READ_TIME read_time
static inline uint64_t read_time(void)
{
union {
struct {
unsigned lo;
unsigned hi;
} p;
unsigned long long c;
} t;
__asm__ volatile ("%0=cycles; %1=cycles2;" : "=d" (t.p.lo), "=d" (t.p.hi));
return t.c;
}
