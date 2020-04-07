



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>


static const unsigned __int64 epoch = 116444736000000000;







int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
(void)tzp;
FILETIME file_time;
SYSTEMTIME system_time;
ULARGE_INTEGER ularge;

GetSystemTime(&system_time);
SystemTimeToFileTime(&system_time, &file_time);
ularge.LowPart = file_time.dwLowDateTime;
ularge.HighPart = file_time.dwHighDateTime;

tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

return 0;
}
#endif 
