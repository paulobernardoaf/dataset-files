#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "libvlc.h"
#include <stdarg.h>
#include <stdatomic.h>
#include <stdnoreturn.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h> 
#include <pthread.h>
#include <sched.h>
#if defined(HAVE_EXECINFO_H)
#include <execinfo.h>
#endif
#if defined(__SunOS)
#include <sys/processor.h>
#include <sys/pset.h>
#endif
static unsigned vlc_clock_prec;
static void vlc_clock_setup_once (void)
{
struct timespec res;
if (unlikely(clock_getres(CLOCK_MONOTONIC, &res) != 0 || res.tv_sec != 0))
abort ();
vlc_clock_prec = (res.tv_nsec + 500) / 1000;
}
void vlc_trace (const char *fn, const char *file, unsigned line)
{
fprintf (stderr, "at %s:%u in %s\n", file, line, fn);
fflush (stderr); 
#if defined(HAVE_BACKTRACE)
void *stack[20];
int len = backtrace (stack, ARRAY_SIZE (stack) );
backtrace_symbols_fd (stack, len, 2);
#endif
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
perror ("Thread error");
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
#define VLC_STACKSIZE (128 * sizeof (void *) * 1024)
#if defined(VLC_STACKSIZE)
ret = pthread_attr_setstacksize (attr, VLC_STACKSIZE);
assert (ret == 0); 
#endif
ret = pthread_create(&th->handle, attr, entry, data);
pthread_sigmask (SIG_SETMASK, &oldset, NULL);
pthread_attr_destroy (attr);
(void) priority;
return ret;
}
int vlc_clone (vlc_thread_t *th, void *(*entry) (void *), void *data,
int priority)
{
pthread_attr_t attr;
pthread_attr_init (&attr);
return vlc_clone_attr (th, &attr, entry, data, priority);
}
void vlc_join(vlc_thread_t th, void **result)
{
int val = pthread_join(th.handle, result);
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
VLC_WEAK unsigned long vlc_thread_id(void)
{
return -1;
}
int vlc_set_priority (vlc_thread_t th, int priority)
{
(void) th; (void) priority;
return VLC_SUCCESS;
}
void vlc_cancel(vlc_thread_t th)
{
pthread_cancel(th.handle);
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
struct timespec ts;
if (unlikely(clock_gettime(CLOCK_MONOTONIC, &ts) != 0))
abort ();
return vlc_tick_from_timespec( &ts );
}
#undef vlc_tick_wait
void vlc_tick_wait (vlc_tick_t deadline)
{
static pthread_once_t vlc_clock_once = PTHREAD_ONCE_INIT;
pthread_once(&vlc_clock_once, vlc_clock_setup_once);
deadline -= vlc_clock_prec;
struct timespec ts = timespec_from_vlc_tick (deadline);
while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL) == EINTR);
}
#undef vlc_tick_sleep
void vlc_tick_sleep (vlc_tick_t delay)
{
struct timespec ts = timespec_from_vlc_tick (delay);
while (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts) == EINTR);
}
unsigned vlc_GetCPUCount(void)
{
#if defined(HAVE_SCHED_GETAFFINITY)
cpu_set_t cpu;
CPU_ZERO(&cpu);
if (sched_getaffinity (0, sizeof (cpu), &cpu) < 0)
return 1;
return CPU_COUNT (&cpu);
#elif defined(__SunOS)
unsigned count = 0;
int type;
u_int numcpus;
processor_info_t cpuinfo;
processorid_t *cpulist = vlc_alloc (sysconf(_SC_NPROCESSORS_MAX), sizeof (*cpulist));
if (unlikely(cpulist == NULL))
return 1;
if (pset_info(PS_MYID, &type, &numcpus, cpulist) == 0)
{
for (u_int i = 0; i < numcpus; i++)
if (processor_info (cpulist[i], &cpuinfo) == 0)
count += (cpuinfo.pi_state == P_ONLINE);
}
else
count = sysconf (_SC_NPROCESSORS_ONLN);
free (cpulist);
return count ? count : 1;
#elif defined(_SC_NPROCESSORS_CONF)
return sysconf(_SC_NPROCESSORS_CONF);
#else
#warning "vlc_GetCPUCount is not implemented for your platform"
return 1;
#endif
}
