#if defined(__APPLE__)
#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <assert.h>
#include <pthread.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <mach/clock_types.h>
int clock_nanosleep(clockid_t clock_id, int flags,
const struct timespec *rqtp, struct timespec *rmtp)
{
if (rqtp == NULL || rqtp->tv_sec < 0 ||
rqtp->tv_nsec < 0 || (unsigned long)rqtp->tv_nsec >= NSEC_PER_SEC) {
errno = EINVAL;
return -1;
}
switch (clock_id) {
case CLOCK_MONOTONIC:
case CLOCK_REALTIME:
break;
default:
errno = EINVAL;
return -1;
}
if (flags == TIMER_ABSTIME) {
struct timespec ts_rel;
struct timespec ts_now;
do {
if (clock_gettime(clock_id, &ts_now) != 0)
return -1;
ts_rel.tv_sec = rqtp->tv_sec - ts_now.tv_sec;
ts_rel.tv_nsec = rqtp->tv_nsec - ts_now.tv_nsec;
if (ts_rel.tv_nsec < 0) {
ts_rel.tv_sec -= 1;
ts_rel.tv_nsec += NSEC_PER_SEC;
}
if (ts_rel.tv_sec < 0 || (ts_rel.tv_sec == 0 && ts_rel.tv_nsec == 0)) {
pthread_testcancel();
return 0;
}
} while (nanosleep(&ts_rel, NULL) == 0);
return -1;
} else if (flags == 0) {
return nanosleep(rqtp, rmtp);
} else {
errno = EINVAL;
return -1;
}
}
#endif
