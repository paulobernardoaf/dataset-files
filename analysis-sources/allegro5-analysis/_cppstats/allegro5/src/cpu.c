#include "allegro5/allegro.h"
#include "allegro5/cpu.h"
#include "allegro5/internal/aintern.h"
#if defined(ALLEGRO_HAVE_SYSCONF)
#include <unistd.h>
#endif
#if defined(ALLEGRO_HAVE_SYSCTL)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#if defined(ALLEGRO_WINDOWS)
#if !defined(WINVER)
#define WINVER 0x0500
#endif
#include <windows.h>
#endif
int al_get_cpu_count(void)
{
#if defined(ALLEGRO_HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
return (int)sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(ALLEGRO_HAVE_SYSCTL)
#if defined(HW_AVAILCPU)
int mib[2] = {CTL_HW, HW_AVAILCPU};
#elif defined(HW_NCPU)
int mib[2] = {CTL_HW, HW_NCPU};
#else
return -1;
#endif
int ncpu = 1;
size_t len = sizeof(ncpu);
if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == 0) { 
return ncpu;
}
#elif defined(ALLEGRO_WINDOWS)
SYSTEM_INFO info;
GetSystemInfo(&info);
return info.dwNumberOfProcessors;
#endif
return -1;
}
int al_get_ram_size(void)
{
#if defined(ALLEGRO_HAVE_SYSCONF) && defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
uint64_t aid = (uint64_t) sysconf(_SC_PHYS_PAGES);
aid *= (uint64_t) sysconf(_SC_PAGESIZE);
aid /= (uint64_t) (1024 * 1024);
return (int)(aid);
#elif defined(ALLEGRO_HAVE_SYSCTL)
#if defined(HW_REALMEM)
int mib[2] = {CTL_HW, HW_REALMEM};
#elif defined(HW_PHYSMEM)
int mib[2] = {CTL_HW, HW_PHYSMEM};
#elif defined(HW_MEMSIZE)
int mib[2] = {CTL_HW, HW_MEMSIZE};
#else
return -1;
#endif
uint64_t memsize = 0;
size_t len = sizeof(memsize);
if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) { 
return (int)(memsize / (1024*1024));
}
#elif defined(ALLEGRO_WINDOWS)
MEMORYSTATUSEX status;
status.dwLength = sizeof(status);
if (GlobalMemoryStatusEx(&status)) {
return (int)(status.ullTotalPhys / (1024 * 1024));
}
#endif
return -1;
}
