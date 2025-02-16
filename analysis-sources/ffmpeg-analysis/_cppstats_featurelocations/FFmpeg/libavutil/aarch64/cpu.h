

















#if !defined(AVUTIL_AARCH64_CPU_H)
#define AVUTIL_AARCH64_CPU_H

#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"

#define have_armv8(flags) CPUEXT(flags, ARMV8)
#define have_neon(flags) CPUEXT(flags, NEON)
#define have_vfp(flags) CPUEXT(flags, VFP)

#endif 
