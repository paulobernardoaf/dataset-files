



































#include "pthread.h"
#include "implement.h"
#if !defined(NEED_FTIME)
#include <sys/timeb.h>
#endif


#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif 
DWORD
ptw32_relmillisecs (const struct timespec * abstime)
{
const int64_t NANOSEC_PER_MILLISEC = 1000000;
const int64_t MILLISEC_PER_SEC = 1000;
DWORD milliseconds;
int64_t tmpAbsMilliseconds;
int64_t tmpCurrMilliseconds;
#if defined(NEED_FTIME)
struct timespec currSysTime;
FILETIME ft;
SYSTEMTIME st;
#else 
#if ( defined(_MSC_VER) && _MSC_VER >= 1300 ) || ( (defined(__MINGW64__) || defined(__MINGW32__)) && __MSVCRT_VERSION__ >= 0x0601 )

struct __timeb64 currSysTime;
#else
struct _timeb currSysTime;
#endif
#endif 













tmpAbsMilliseconds = (int64_t)abstime->tv_sec * MILLISEC_PER_SEC;
tmpAbsMilliseconds += ((int64_t)abstime->tv_nsec + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC;



#if defined(NEED_FTIME)

GetSystemTime(&st);
SystemTimeToFileTime(&st, &ft);





ptw32_filetime_to_timespec(&ft, &currSysTime);

tmpCurrMilliseconds = (int64_t)currSysTime.tv_sec * MILLISEC_PER_SEC;
tmpCurrMilliseconds += ((int64_t)currSysTime.tv_nsec + (NANOSEC_PER_MILLISEC/2))
/ NANOSEC_PER_MILLISEC;

#else 

#if defined(_MSC_VER) && _MSC_VER >= 1400
_ftime64_s(&currSysTime);
#elif ( defined(_MSC_VER) && _MSC_VER >= 1300 ) || ( (defined(__MINGW64__) || defined(__MINGW32__)) && __MSVCRT_VERSION__ >= 0x0601 )

_ftime64(&currSysTime);
#else
_ftime(&currSysTime);
#endif

tmpCurrMilliseconds = (int64_t) currSysTime.time * MILLISEC_PER_SEC;
tmpCurrMilliseconds += (int64_t) currSysTime.millitm;

#endif 

if (tmpAbsMilliseconds > tmpCurrMilliseconds)
{
milliseconds = (DWORD) (tmpAbsMilliseconds - tmpCurrMilliseconds);
if (milliseconds == INFINITE)
{

milliseconds--;
}
}
else
{

milliseconds = 0;
}

return milliseconds;
}
