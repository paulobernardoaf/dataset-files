

















#if !defined(AVUTIL_PPC_CPU_H)
#define AVUTIL_PPC_CPU_H

#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"

#define PPC_ALTIVEC(flags) CPUEXT(flags, ALTIVEC)
#define PPC_VSX(flags) CPUEXT(flags, VSX)
#define PPC_POWER8(flags) CPUEXT(flags, POWER8)

#endif 
