#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#if !defined(FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE FUTEX_WAKE
#define FUTEX_WAIT_PRIVATE FUTEX_WAIT
#define FUTEX_WAIT_BITSET_PRIVATE FUTEX_WAIT_BITSET
#endif
#include <vlc_common.h>
unsigned long vlc_thread_id(void)
{
static __thread pid_t tid = 0;
if (unlikely(tid == 0))
tid = syscall(__NR_gettid);
return tid;
}
static int sys_futex(void *addr, int op, unsigned val,
const struct timespec *to, void *addr2, int val3)
{
return syscall(__NR_futex, addr, op, val, to, addr2, val3);
}
static int vlc_futex_wake(void *addr, int nr)
{
return sys_futex(addr, FUTEX_WAKE_PRIVATE, nr, NULL, NULL, 0);
}
static int vlc_futex_wait(void *addr, unsigned flags,
unsigned val, const struct timespec *to)
{
int ret;
#if !defined(__ANDROID__)
int type;
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &type);
#endif
ret = sys_futex(addr, FUTEX_WAIT_BITSET_PRIVATE | flags, val, to, NULL,
FUTEX_BITSET_MATCH_ANY);
#if !defined(__ANDROID__)
pthread_setcanceltype(type, NULL);
#endif
return ret;
}
void vlc_atomic_notify_one(void *addr)
{
vlc_futex_wake(addr, 1);
}
void vlc_atomic_notify_all(void *addr)
{
vlc_futex_wake(addr, INT_MAX);
}
void vlc_atomic_wait(void *addr, unsigned val)
{
vlc_futex_wait(addr, 0, val, NULL);
}
int vlc_atomic_timedwait(void *addr, unsigned val, vlc_tick_t deadline)
{
struct timespec ts = timespec_from_vlc_tick(deadline);
if (vlc_futex_wait(addr, 0, val, &ts) == 0)
return 0;
switch (errno) {
case EINTR:
case EAGAIN:
return 0;
case EFAULT:
case EINVAL:
vlc_assert_unreachable(); 
default:
break;
}
return errno;
}
int vlc_atomic_timedwait_daytime(void *addr, unsigned val, time_t deadline)
{
struct timespec ts = { .tv_sec = deadline, .tv_nsec = 0 };
if (vlc_futex_wait(addr, FUTEX_CLOCK_REALTIME, val, &ts) == 0)
return 0;
switch (errno) {
case EINTR:
case EAGAIN:
return 0;
case EFAULT:
case EINVAL:
vlc_assert_unreachable(); 
default:
break;
}
return errno;
}
