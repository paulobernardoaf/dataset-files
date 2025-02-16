#if defined(__APPLE__)
#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mach/clock_types.h>
int clock_getres(clockid_t clock_id, struct timespec *tp)
{
switch (clock_id) {
case CLOCK_MONOTONIC:
case CLOCK_REALTIME:
tp->tv_sec = 0;
tp->tv_nsec = NSEC_PER_USEC;
break;
default:
errno = EINVAL;
return -1;
}
return 0;
}
#endif
