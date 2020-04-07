

















#if !defined(AVUTIL_CPU_INTERNAL_H)
#define AVUTIL_CPU_INTERNAL_H

#include "config.h"

#include "cpu.h"

#define CPUEXT_SUFFIX(flags, suffix, cpuext) (HAVE_ ##cpuext ##suffix && ((flags) & AV_CPU_FLAG_ ##cpuext))


#define CPUEXT_SUFFIX_FAST2(flags, suffix, cpuext, slow_cpuext) (HAVE_ ##cpuext ##suffix && ((flags) & AV_CPU_FLAG_ ##cpuext) && !((flags) & AV_CPU_FLAG_ ##slow_cpuext ##SLOW))



#define CPUEXT_SUFFIX_SLOW2(flags, suffix, cpuext, slow_cpuext) (HAVE_ ##cpuext ##suffix && ((flags) & AV_CPU_FLAG_ ##cpuext) && ((flags) & AV_CPU_FLAG_ ##slow_cpuext ##SLOW))



#define CPUEXT_SUFFIX_FAST(flags, suffix, cpuext) CPUEXT_SUFFIX_FAST2(flags, suffix, cpuext, cpuext)
#define CPUEXT_SUFFIX_SLOW(flags, suffix, cpuext) CPUEXT_SUFFIX_SLOW2(flags, suffix, cpuext, cpuext)

#define CPUEXT(flags, cpuext) CPUEXT_SUFFIX(flags, , cpuext)
#define CPUEXT_FAST(flags, cpuext) CPUEXT_SUFFIX_FAST(flags, , cpuext)
#define CPUEXT_SLOW(flags, cpuext) CPUEXT_SUFFIX_SLOW(flags, , cpuext)

int ff_get_cpu_flags_aarch64(void);
int ff_get_cpu_flags_arm(void);
int ff_get_cpu_flags_ppc(void);
int ff_get_cpu_flags_x86(void);

size_t ff_get_cpu_max_align_aarch64(void);
size_t ff_get_cpu_max_align_arm(void);
size_t ff_get_cpu_max_align_ppc(void);
size_t ff_get_cpu_max_align_x86(void);

#endif 
