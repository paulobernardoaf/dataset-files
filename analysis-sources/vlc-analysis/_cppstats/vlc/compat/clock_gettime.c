#if defined(__APPLE__)
#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysctl.h>
static int vlc__get_system_boottime(struct timeval *tv)
{
int ret = sysctl((int[]){ CTL_KERN, KERN_BOOTTIME }, 2,
tv, &(size_t){ sizeof(*tv) }, NULL, 0);
if (ret != 0)
return errno;
return 0;
}
static int vlc__get_monotonic(struct timeval *tv)
{
int ret;
struct timeval currenttime;
struct timeval boottime_begin;
struct timeval boottime_end;
do {
ret = vlc__get_system_boottime(&boottime_begin);
if (ret != 0)
return ret;
ret = gettimeofday(&currenttime, NULL);
if (ret != 0)
return ret;
ret = vlc__get_system_boottime(&boottime_end);
if (ret != 0)
return ret;
} while (timercmp(&boottime_begin, &boottime_end, !=));
timersub(&currenttime, &boottime_begin, tv);
return 0;
}
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
int ret = 0;
struct timeval tv;
switch (clock_id) {
case CLOCK_MONOTONIC:
ret = vlc__get_monotonic(&tv);
break;
case CLOCK_REALTIME:
ret = gettimeofday(&tv, NULL);
break;
default:
errno = EINVAL;
return -1;
}
if (ret == 0)
TIMEVAL_TO_TIMESPEC(&tv, tp);
return ret;
}
#endif
