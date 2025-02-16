#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <stdatomic.h>
#include "libvlc.h"
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <mach/mach_init.h> 
#include <mach/mach_time.h>
#include <execinfo.h>
static struct {
uint32_t quotient;
uint32_t remainder;
uint32_t divider;
} vlc_clock_conversion;
static void vlc_clock_setup_once (void)
{
mach_timebase_info_data_t timebase;
if (unlikely(mach_timebase_info (&timebase) != 0))
abort ();
lldiv_t d = lldiv (timebase.numer, timebase.denom);
vlc_clock_conversion.quotient = (uint32_t)d.quot;
vlc_clock_conversion.remainder = (uint32_t)d.rem;
vlc_clock_conversion.divider = timebase.denom;
}
static pthread_once_t vlc_clock_once = PTHREAD_ONCE_INIT;
#define vlc_clock_setup() pthread_once(&vlc_clock_once, vlc_clock_setup_once)
void vlc_trace (const char *fn, const char *file, unsigned line)
{
fprintf (stderr, "at %s:%u in %s\n", file, line, fn);
fflush (stderr); 
void *stack[20];
int len = backtrace (stack, sizeof (stack) / sizeof (stack[0]));
backtrace_symbols_fd (stack, len, 2);
fsync (2);
}
#if !defined(NDEBUG)
static void
vlc_thread_fatal (const char *action, int error,
const char *function, const char *file, unsigned line)
{
int canc = vlc_savecancel ();
fprintf (stderr, "LibVLC fatal error %s (%d) in thread %lu ",
action, error, vlc_thread_id ());
vlc_trace (function, file, line);
char buf[1000];
const char *msg;
switch (strerror_r (error, buf, sizeof (buf)))
{
case 0:
msg = buf;
break;
case ERANGE: 
msg = "unknown (too big to display)";
break;
default:
msg = "unknown (invalid error number)";
break;
}
fprintf (stderr, " Error message: %s\n", msg);
fflush (stderr);
vlc_restorecancel (canc);
abort ();
}
#define VLC_THREAD_ASSERT( action ) if (unlikely(val)) vlc_thread_fatal (action, val, __func__, __FILE__, __LINE__)
#else
#define VLC_THREAD_ASSERT( action ) ((void)val)
#endif
void vlc_rwlock_init (vlc_rwlock_t *lock)
{
if (unlikely(pthread_rwlock_init (lock, NULL)))
abort ();
}
void vlc_rwlock_destroy (vlc_rwlock_t *lock)
{
int val = pthread_rwlock_destroy (lock);
VLC_THREAD_ASSERT ("destroying R/W lock");
}
void vlc_rwlock_rdlock (vlc_rwlock_t *lock)
{
int val = pthread_rwlock_rdlock (lock);
VLC_THREAD_ASSERT ("acquiring R/W lock for reading");
}
void vlc_rwlock_wrlock (vlc_rwlock_t *lock)
{
int val = pthread_rwlock_wrlock (lock);
VLC_THREAD_ASSERT ("acquiring R/W lock for writing");
}
void vlc_rwlock_unlock (vlc_rwlock_t *lock)
{
int val = pthread_rwlock_unlock (lock);
VLC_THREAD_ASSERT ("releasing R/W lock");
}
int vlc_threadvar_create (vlc_threadvar_t *key, void (*destr) (void *))
{
return pthread_key_create (key, destr);
}
void vlc_threadvar_delete (vlc_threadvar_t *p_tls)
{
pthread_key_delete (*p_tls);
}
int vlc_threadvar_set (vlc_threadvar_t key, void *value)
{
return pthread_setspecific (key, value);
}
void *vlc_threadvar_get (vlc_threadvar_t key)
{
return pthread_getspecific (key);
}
void vlc_threads_setup (libvlc_int_t *p_libvlc)
{
(void) p_libvlc;
}
static int vlc_clone_attr (vlc_thread_t *th, pthread_attr_t *attr,
void *(*entry) (void *), void *data, int priority)
{
int ret;
sigset_t oldset;
{
sigset_t set;
sigemptyset (&set);
sigdelset (&set, SIGHUP);
sigaddset (&set, SIGINT);
sigaddset (&set, SIGQUIT);
sigaddset (&set, SIGTERM);
sigaddset (&set, SIGPIPE); 
pthread_sigmask (SIG_BLOCK, &set, &oldset);
}
(void) priority;
#define VLC_STACKSIZE (128 * sizeof (void *) * 1024)
#if defined(VLC_STACKSIZE)
ret = pthread_attr_setstacksize (attr, VLC_STACKSIZE);
assert (ret == 0); 
#endif
ret = pthread_create (th, attr, entry, data);
pthread_sigmask (SIG_SETMASK, &oldset, NULL);
pthread_attr_destroy (attr);
return ret;
}
int vlc_clone (vlc_thread_t *th, void *(*entry) (void *), void *data,
int priority)
{
pthread_attr_t attr;
pthread_attr_init (&attr);
return vlc_clone_attr (th, &attr, entry, data, priority);
}
void vlc_join (vlc_thread_t handle, void **result)
{
int val = pthread_join (handle, result);
VLC_THREAD_ASSERT ("joining thread");
}
int vlc_clone_detach (vlc_thread_t *th, void *(*entry) (void *), void *data,
int priority)
{
vlc_thread_t dummy;
pthread_attr_t attr;
if (th == NULL)
th = &dummy;
pthread_attr_init (&attr);
pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
return vlc_clone_attr (th, &attr, entry, data, priority);
}
unsigned long vlc_thread_id (void)
{
return -1;
}
int vlc_set_priority (vlc_thread_t th, int priority)
{
(void) th; (void) priority;
return VLC_SUCCESS;
}
void vlc_cancel (vlc_thread_t thread_id)
{
pthread_cancel (thread_id);
}
int vlc_savecancel (void)
{
int state;
int val = pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &state);
VLC_THREAD_ASSERT ("saving cancellation");
return state;
}
void vlc_restorecancel (int state)
{
#if !defined(NDEBUG)
int oldstate, val;
val = pthread_setcancelstate (state, &oldstate);
VLC_THREAD_ASSERT ("restoring cancellation");
if (unlikely(oldstate != PTHREAD_CANCEL_DISABLE))
vlc_thread_fatal ("restoring cancellation while not disabled", EINVAL,
__func__, __FILE__, __LINE__);
#else
pthread_setcancelstate (state, NULL);
#endif
}
void vlc_testcancel (void)
{
pthread_testcancel ();
}
vlc_tick_t vlc_tick_now (void)
{
vlc_clock_setup();
uint64_t date = mach_absolute_time();
date = date * vlc_clock_conversion.quotient +
date * vlc_clock_conversion.remainder / vlc_clock_conversion.divider;
return VLC_TICK_FROM_NS(date);
}
#undef vlc_tick_wait
void vlc_tick_wait (vlc_tick_t deadline)
{
deadline -= vlc_tick_now ();
if (deadline > 0)
vlc_tick_sleep (deadline);
}
#undef vlc_tick_sleep
void vlc_tick_sleep (vlc_tick_t delay)
{
struct timespec ts = timespec_from_vlc_tick (delay);
while (nanosleep (&ts, &ts) == -1)
assert (errno == EINTR);
}
unsigned vlc_GetCPUCount(void)
{
return sysconf(_SC_NPROCESSORS_CONF);
}
