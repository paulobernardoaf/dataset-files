











#include "tuklib_cpucores.h"

#if defined(TUKLIB_CPUCORES_SYSCTL)
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#include <sys/sysctl.h>

#elif defined(TUKLIB_CPUCORES_SYSCONF)
#include <unistd.h>


#elif defined(TUKLIB_CPUCORES_PSTAT_GETDYNAMIC)
#include <sys/param.h>
#include <sys/pstat.h>
#endif


extern uint32_t
tuklib_cpucores(void)
{
uint32_t ret = 0;

#if defined(TUKLIB_CPUCORES_SYSCTL)
int name[2] = { CTL_HW, HW_NCPU };
int cpus;
size_t cpus_size = sizeof(cpus);
if (sysctl(name, 2, &cpus, &cpus_size, NULL, 0) != -1
&& cpus_size == sizeof(cpus) && cpus > 0)
ret = cpus;

#elif defined(TUKLIB_CPUCORES_SYSCONF)
#if defined(_SC_NPROCESSORS_ONLN)

const long cpus = sysconf(_SC_NPROCESSORS_ONLN);
#else

const long cpus = sysconf(_SC_NPROC_ONLN);
#endif
if (cpus > 0)
ret = cpus;

#elif defined(TUKLIB_CPUCORES_PSTAT_GETDYNAMIC)
struct pst_dynamic pst;
if (pstat_getdynamic(&pst, sizeof(pst), 1, 0) != -1)
ret = pst.psd_proc_cnt;
#endif

return ret;
}
