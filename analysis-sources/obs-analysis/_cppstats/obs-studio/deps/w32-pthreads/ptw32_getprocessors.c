#include "pthread.h"
#include "implement.h"
int
ptw32_getprocessors (int *count)
{
DWORD_PTR vProcessCPUs;
DWORD_PTR vSystemCPUs;
int result = 0;
#if defined(NEED_PROCESS_AFFINITY_MASK)
*count = 1;
#else
if (GetProcessAffinityMask (GetCurrentProcess (),
&vProcessCPUs, &vSystemCPUs))
{
DWORD_PTR bit;
int CPUs = 0;
for (bit = 1; bit != 0; bit <<= 1)
{
if (vProcessCPUs & bit)
{
CPUs++;
}
}
*count = CPUs;
}
else
{
result = EAGAIN;
}
#endif
return (result);
}
