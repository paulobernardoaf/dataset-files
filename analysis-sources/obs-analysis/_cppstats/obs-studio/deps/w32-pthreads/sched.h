#undef PTW32_SCHED_LEVEL
#if defined(_POSIX_SOURCE)
#define PTW32_SCHED_LEVEL 0
#endif
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309
#undef PTW32_SCHED_LEVEL
#define PTW32_SCHED_LEVEL 1
#endif
#if defined(INCLUDE_NP)
#undef PTW32_SCHED_LEVEL
#define PTW32_SCHED_LEVEL 2
#endif
#define PTW32_SCHED_LEVEL_MAX 3
#if ( defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112 ) || !defined(PTW32_SCHED_LEVEL)
#define PTW32_SCHED_LEVEL PTW32_SCHED_LEVEL_MAX
#endif
#if defined(__GNUC__) && !defined(__declspec)
#error Please upgrade your GNU compiler to one that supports __declspec.
#endif
#if !defined(PTW32_STATIC_LIB)
#if defined(PTW32_BUILD)
#define PTW32_DLLPORT __declspec (dllexport)
#else
#define PTW32_DLLPORT
#endif
#else
#define PTW32_DLLPORT
#endif
#if !defined(PTW32_CONFIG_H)
#if defined(WINCE)
#define NEED_ERRNO
#define NEED_SEM
#endif
#if defined(__MINGW64__)
#define HAVE_STRUCT_TIMESPEC
#define HAVE_MODE_T
#elif defined(_UWIN) || defined(__MINGW32__)
#define HAVE_MODE_T
#endif
#endif
#if PTW32_SCHED_LEVEL >= PTW32_SCHED_LEVEL_MAX
#if defined(NEED_ERRNO)
#include "need_errno.h"
#else
#include <errno.h>
#endif
#endif 
#if (defined(__MINGW64__) || defined(__MINGW32__)) || defined(_UWIN)
#if PTW32_SCHED_LEVEL >= PTW32_SCHED_LEVEL_MAX
#include <sys/types.h>
#include <time.h>
#else
typedef int pid_t;
#endif
#else
typedef int pid_t;
#endif
enum {
SCHED_OTHER = 0,
SCHED_FIFO,
SCHED_RR,
SCHED_MIN = SCHED_OTHER,
SCHED_MAX = SCHED_RR
};
struct sched_param {
int sched_priority;
};
#if defined(__cplusplus)
extern "C"
{
#endif 
PTW32_DLLPORT int __cdecl sched_yield (void);
PTW32_DLLPORT int __cdecl sched_get_priority_min (int policy);
PTW32_DLLPORT int __cdecl sched_get_priority_max (int policy);
PTW32_DLLPORT int __cdecl sched_setscheduler (pid_t pid, int policy);
PTW32_DLLPORT int __cdecl sched_getscheduler (pid_t pid);
#define sched_rr_get_interval(_pid, _interval) ( errno = ENOTSUP, (int) -1 )
#if defined(__cplusplus)
} 
#endif 
#undef PTW32_SCHED_LEVEL
#undef PTW32_SCHED_LEVEL_MAX
