#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/thr.h>
#include <sys/umtx.h>
#include <vlc_common.h>
unsigned long vlc_thread_id(void)
{
static _Thread_local int tid = -1;
if (unlikely(tid == -1))
tid = thr_self();
return tid;
}
static int vlc_umtx_wake(void *addr, int nr)
{
return _umtx_op(addr, UMTX_OP_WAKE_PRIVATE, nr, NULL, NULL);
}
static int vlc_umtx_wait(void *addr, unsigned val, const struct timespec *ts)
{
struct _umtx_time to = {
._timeout = *ts,
._flags = UMTX_ABSTIME,
._clockid = CLOCK_MONOTONIC,
};
int ret, type;
void *uaddr = (void *)sizeof (to);
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &type);
ret = _umtx_op(addr, UMTX_OP_WAIT_UINT_PRIVATE, val, uaddr, &to);
pthread_setcanceltype(type, NULL);
return ret;
}
void vlc_atomic_notify_one(void *addr)
{
vlc_umtx_wake(addr, 1);
}
void vlc_atomic_notify_all(void *addr)
{
vlc_umtx_wake(addr, INT_MAX);
}
void vlc_atomic_wait(void *addr, unsigned val)
{
int ret = vlc_umtx_wait(addr, val, NULL);
assert(ret == 0 || ret == EINTR || ret == ERESTART);
}
int vlc_atomic_timedwait(void *addr, unsigned val, vlc_tick_t deadline)
{
struct timespec ts = timespec_from_vlc_tick(delay);
int ret = vlc_umtx_wait(addr, val, &ts);
assert(ret == 0 || ret == ETIMEDOUT || ret == EINTR || ret == ERESTART);
return (ret != ETIMEDOUT) ? 0 : ret;
}
