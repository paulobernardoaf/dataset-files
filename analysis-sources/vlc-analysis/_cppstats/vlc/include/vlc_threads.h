#if !defined(__cplusplus)
#include <stdatomic.h>
#endif
VLC_API void vlc_testcancel(void);
#if defined (_WIN32)
#include <process.h>
#if !defined(ETIMEDOUT)
#define ETIMEDOUT 10060 
#endif
typedef struct vlc_thread *vlc_thread_t;
#define VLC_THREAD_CANCELED NULL
#define LIBVLC_NEED_SLEEP
#define LIBVLC_NEED_RWLOCK
typedef struct vlc_threadvar *vlc_threadvar_t;
typedef struct vlc_timer *vlc_timer_t;
#define VLC_THREAD_PRIORITY_LOW 0
#define VLC_THREAD_PRIORITY_INPUT THREAD_PRIORITY_ABOVE_NORMAL
#define VLC_THREAD_PRIORITY_AUDIO THREAD_PRIORITY_HIGHEST
#define VLC_THREAD_PRIORITY_VIDEO 0
#define VLC_THREAD_PRIORITY_OUTPUT THREAD_PRIORITY_ABOVE_NORMAL
#define VLC_THREAD_PRIORITY_HIGHEST THREAD_PRIORITY_TIME_CRITICAL
static inline int vlc_poll(struct pollfd *fds, unsigned nfds, int timeout)
{
int val;
vlc_testcancel();
val = poll(fds, nfds, timeout);
if (val < 0)
vlc_testcancel();
return val;
}
#define poll(u,n,t) vlc_poll(u, n, t)
#elif defined (__OS2__)
#include <errno.h>
typedef struct vlc_thread *vlc_thread_t;
#define VLC_THREAD_CANCELED NULL
#define LIBVLC_NEED_RWLOCK
typedef struct vlc_threadvar *vlc_threadvar_t;
typedef struct vlc_timer *vlc_timer_t;
#define VLC_THREAD_PRIORITY_LOW 0
#define VLC_THREAD_PRIORITY_INPUT MAKESHORT(PRTYD_MAXIMUM / 2, PRTYC_REGULAR)
#define VLC_THREAD_PRIORITY_AUDIO MAKESHORT(PRTYD_MAXIMUM, PRTYC_REGULAR)
#define VLC_THREAD_PRIORITY_VIDEO 0
#define VLC_THREAD_PRIORITY_OUTPUT MAKESHORT(PRTYD_MAXIMUM / 2, PRTYC_REGULAR)
#define VLC_THREAD_PRIORITY_HIGHEST MAKESHORT(0, PRTYC_TIMECRITICAL)
#define pthread_sigmask sigprocmask
static inline int vlc_poll (struct pollfd *fds, unsigned nfds, int timeout)
{
static int (*vlc_poll_os2)(struct pollfd *, unsigned, int) = NULL;
if (!vlc_poll_os2)
{
HMODULE hmod;
CHAR szFailed[CCHMAXPATH];
if (DosLoadModule(szFailed, sizeof(szFailed), "vlccore", &hmod))
return -1;
if (DosQueryProcAddr(hmod, 0, "_vlc_poll_os2", (PFN *)&vlc_poll_os2))
return -1;
}
return (*vlc_poll_os2)(fds, nfds, timeout);
}
#define poll(u,n,t) vlc_poll(u, n, t)
#elif defined (__ANDROID__) 
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#define LIBVLC_USE_PTHREAD_CLEANUP 1
#define LIBVLC_NEED_SLEEP
#define LIBVLC_NEED_RWLOCK
typedef struct vlc_thread *vlc_thread_t;
#define VLC_THREAD_CANCELED NULL
typedef pthread_key_t vlc_threadvar_t;
typedef struct vlc_timer *vlc_timer_t;
#define VLC_THREAD_PRIORITY_LOW 0
#define VLC_THREAD_PRIORITY_INPUT 0
#define VLC_THREAD_PRIORITY_AUDIO 0
#define VLC_THREAD_PRIORITY_VIDEO 0
#define VLC_THREAD_PRIORITY_OUTPUT 0
#define VLC_THREAD_PRIORITY_HIGHEST 0
static inline int vlc_poll (struct pollfd *fds, unsigned nfds, int timeout)
{
int val;
do
{
int ugly_timeout = ((unsigned)timeout >= 50) ? 50 : timeout;
if (timeout >= 0)
timeout -= ugly_timeout;
vlc_testcancel ();
val = poll (fds, nfds, ugly_timeout);
}
while (val == 0 && timeout != 0);
return val;
}
#define poll(u,n,t) vlc_poll(u, n, t)
#elif defined (__APPLE__)
#define _APPLE_C_SOURCE 1 
#include <unistd.h>
#include <pthread.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#define LIBVLC_USE_PTHREAD_CLEANUP 1
typedef pthread_t vlc_thread_t;
#define VLC_THREAD_CANCELED PTHREAD_CANCELED
typedef pthread_rwlock_t vlc_rwlock_t;
#define VLC_STATIC_RWLOCK PTHREAD_RWLOCK_INITIALIZER
typedef pthread_key_t vlc_threadvar_t;
typedef struct vlc_timer *vlc_timer_t;
#define VLC_THREAD_PRIORITY_LOW 0
#define VLC_THREAD_PRIORITY_INPUT 22
#define VLC_THREAD_PRIORITY_AUDIO 22
#define VLC_THREAD_PRIORITY_VIDEO 0
#define VLC_THREAD_PRIORITY_OUTPUT 22
#define VLC_THREAD_PRIORITY_HIGHEST 22
#else 
#include <unistd.h> 
#include <pthread.h>
#define LIBVLC_USE_PTHREAD 1
#define LIBVLC_USE_PTHREAD_CLEANUP 1
typedef struct
{
pthread_t handle;
} vlc_thread_t;
#define VLC_THREAD_CANCELED PTHREAD_CANCELED
typedef pthread_rwlock_t vlc_rwlock_t;
#define VLC_STATIC_RWLOCK PTHREAD_RWLOCK_INITIALIZER
typedef pthread_key_t vlc_threadvar_t;
typedef struct vlc_timer *vlc_timer_t;
#define VLC_THREAD_PRIORITY_LOW 0
#define VLC_THREAD_PRIORITY_INPUT 10
#define VLC_THREAD_PRIORITY_AUDIO 5
#define VLC_THREAD_PRIORITY_VIDEO 0
#define VLC_THREAD_PRIORITY_OUTPUT 15
#define VLC_THREAD_PRIORITY_HIGHEST 20
#endif
typedef struct
{
union {
#if !defined(__cplusplus)
struct {
atomic_uint value;
atomic_uint recursion;
_Atomic (const void *) owner;
};
#endif
struct {
unsigned int value;
unsigned int recursion;
const void *owner;
} dummy;
};
} vlc_mutex_t;
#define VLC_STATIC_MUTEX { .value = ATOMIC_VAR_INIT(0), .recursion = ATOMIC_VAR_INIT(0), .owner = ATOMIC_VAR_INIT(NULL), }
VLC_API void vlc_mutex_init(vlc_mutex_t *);
VLC_API void vlc_mutex_init_recursive(vlc_mutex_t *);
VLC_API void vlc_mutex_lock(vlc_mutex_t *);
VLC_API int vlc_mutex_trylock( vlc_mutex_t * ) VLC_USED;
VLC_API void vlc_mutex_unlock(vlc_mutex_t *);
VLC_API bool vlc_mutex_held(const vlc_mutex_t *) VLC_USED;
#define vlc_mutex_assert(m) assert(vlc_mutex_held(m))
struct vlc_cond_waiter;
typedef struct
{
struct vlc_cond_waiter *head;
vlc_mutex_t lock;
} vlc_cond_t;
#define VLC_STATIC_COND { NULL, VLC_STATIC_MUTEX }
VLC_API void vlc_cond_init(vlc_cond_t *);
VLC_API void vlc_cond_signal(vlc_cond_t *);
VLC_API void vlc_cond_broadcast(vlc_cond_t *);
VLC_API void vlc_cond_wait(vlc_cond_t *cond, vlc_mutex_t *mutex);
VLC_API int vlc_cond_timedwait(vlc_cond_t *cond, vlc_mutex_t *mutex,
vlc_tick_t deadline);
int vlc_cond_timedwait_daytime(vlc_cond_t *, vlc_mutex_t *, time_t);
typedef struct
{
union {
#if !defined(__cplusplus)
atomic_uint value;
#endif
int dummy;
};
} vlc_sem_t;
VLC_API void vlc_sem_init(vlc_sem_t *, unsigned count);
VLC_API int vlc_sem_post(vlc_sem_t *);
VLC_API void vlc_sem_wait(vlc_sem_t *);
VLC_API int vlc_sem_timedwait(vlc_sem_t *sem, vlc_tick_t deadline) VLC_USED;
#if defined(LIBVLC_NEED_RWLOCK)
typedef struct vlc_rwlock
{
vlc_mutex_t mutex;
vlc_cond_t wait;
long state;
} vlc_rwlock_t;
#define VLC_STATIC_RWLOCK { VLC_STATIC_MUTEX, VLC_STATIC_COND, 0 }
#endif
VLC_API void vlc_rwlock_init(vlc_rwlock_t *);
VLC_API void vlc_rwlock_destroy(vlc_rwlock_t *);
VLC_API void vlc_rwlock_rdlock(vlc_rwlock_t *);
VLC_API void vlc_rwlock_wrlock(vlc_rwlock_t *);
VLC_API void vlc_rwlock_unlock(vlc_rwlock_t *);
#if !defined(__cplusplus)
typedef struct
{
atomic_uint value;
} vlc_once_t;
#define VLC_STATIC_ONCE { ATOMIC_VAR_INIT(0) }
VLC_API void vlc_once(vlc_once_t *restrict once, void (*cb)(void));
static inline void vlc_once_inline(vlc_once_t *restrict once, void (*cb)(void))
{
if (unlikely(atomic_load_explicit(&once->value, memory_order_acquire) < 3))
vlc_once(once, cb);
}
#define vlc_once(once, cb) vlc_once_inline(once, cb)
#endif
VLC_API int vlc_threadvar_create(vlc_threadvar_t *key, void (*destr) (void *));
VLC_API void vlc_threadvar_delete(vlc_threadvar_t *);
VLC_API int vlc_threadvar_set(vlc_threadvar_t key, void *value);
VLC_API void *vlc_threadvar_get(vlc_threadvar_t);
void vlc_atomic_wait(void *addr, unsigned val);
int vlc_atomic_timedwait(void *addr, unsigned val, vlc_tick_t deadline);
int vlc_atomic_timedwait_daytime(void *addr, unsigned val, time_t deadline);
void vlc_atomic_notify_one(void *addr);
void vlc_atomic_notify_all(void *addr);
VLC_API int vlc_clone(vlc_thread_t *th, void *(*entry)(void *), void *data,
int priority) VLC_USED;
VLC_API void vlc_cancel(vlc_thread_t);
VLC_API void vlc_join(vlc_thread_t th, void **result);
VLC_API int vlc_savecancel(void);
VLC_API void vlc_restorecancel(int state);
typedef struct vlc_cleanup_t vlc_cleanup_t;
VLC_API void vlc_control_cancel(vlc_cleanup_t *);
VLC_API unsigned long vlc_thread_id(void) VLC_USED;
VLC_API vlc_tick_t vlc_tick_now(void);
VLC_API void vlc_tick_wait(vlc_tick_t deadline);
VLC_API void vlc_tick_sleep(vlc_tick_t delay);
#define VLC_HARD_MIN_SLEEP VLC_TICK_FROM_MS(10) 
#define VLC_SOFT_MIN_SLEEP VLC_TICK_FROM_SEC(9) 
#if defined (__GNUC__) && !defined (__clang__)
static
__attribute__((unused))
__attribute__((noinline))
__attribute__((error("sorry, cannot sleep for such short a time")))
vlc_tick_t impossible_delay( vlc_tick_t delay )
{
(void) delay;
return VLC_HARD_MIN_SLEEP;
}
static
__attribute__((unused))
__attribute__((noinline))
__attribute__((warning("use proper event handling instead of short delay")))
vlc_tick_t harmful_delay( vlc_tick_t delay )
{
return delay;
}
#define check_delay( d ) ((__builtin_constant_p(d < VLC_HARD_MIN_SLEEP) && (d < VLC_HARD_MIN_SLEEP)) ? impossible_delay(d) : ((__builtin_constant_p(d < VLC_SOFT_MIN_SLEEP) && (d < VLC_SOFT_MIN_SLEEP)) ? harmful_delay(d) : d))
static
__attribute__((unused))
__attribute__((noinline))
__attribute__((error("deadlines can not be constant")))
vlc_tick_t impossible_deadline( vlc_tick_t deadline )
{
return deadline;
}
#define check_deadline( d ) (__builtin_constant_p(d) ? impossible_deadline(d) : d)
#else
#define check_delay(d) (d)
#define check_deadline(d) (d)
#endif
#define vlc_tick_sleep(d) vlc_tick_sleep(check_delay(d))
#define vlc_tick_wait(d) vlc_tick_wait(check_deadline(d))
VLC_API int vlc_timer_create(vlc_timer_t *id, void (*func)(void *), void *data)
VLC_USED;
VLC_API void vlc_timer_destroy(vlc_timer_t timer);
#define VLC_TIMER_DISARM (0)
#define VLC_TIMER_FIRE_ONCE (0)
VLC_API void vlc_timer_schedule(vlc_timer_t timer, bool absolute,
vlc_tick_t value, vlc_tick_t interval);
static inline void vlc_timer_disarm(vlc_timer_t timer)
{
vlc_timer_schedule( timer, false, VLC_TIMER_DISARM, 0 );
}
static inline void vlc_timer_schedule_asap(vlc_timer_t timer, vlc_tick_t interval)
{
vlc_timer_schedule(timer, false, 1, interval);
}
VLC_API unsigned vlc_timer_getoverrun(vlc_timer_t) VLC_USED;
VLC_API unsigned vlc_GetCPUCount(void);
#if defined (LIBVLC_USE_PTHREAD_CLEANUP)
#define vlc_cleanup_push( routine, arg ) pthread_cleanup_push (routine, arg)
#define vlc_cleanup_pop( ) pthread_cleanup_pop (0)
#else 
struct vlc_cleanup_t
{
vlc_cleanup_t *next;
void (*proc) (void *);
void *data;
};
#if !defined(__cplusplus)
#define vlc_cleanup_push( routine, arg ) do { vlc_control_cancel(&(vlc_cleanup_t){ NULL, routine, arg })
#define vlc_cleanup_pop( ) vlc_control_cancel (NULL); } while (0)
#else
#define vlc_cleanup_push(routine, arg) static_assert(false, "don't use vlc_cleanup_push in portable C++ code")
#define vlc_cleanup_pop() static_assert(false, "don't use vlc_cleanup_pop in portable C++ code")
#endif
#endif 
static inline void vlc_cleanup_lock (void *lock)
{
vlc_mutex_unlock ((vlc_mutex_t *)lock);
}
#define mutex_cleanup_push( lock ) vlc_cleanup_push (vlc_cleanup_lock, lock)
#if !defined(__cplusplus)
void vlc_cancel_addr_set(atomic_uint *addr);
void vlc_cancel_addr_clear(atomic_uint *addr);
#else
class vlc_mutex_locker
{
private:
vlc_mutex_t *lock;
public:
vlc_mutex_locker (vlc_mutex_t *m) : lock (m)
{
vlc_mutex_lock (lock);
}
~vlc_mutex_locker (void)
{
vlc_mutex_unlock (lock);
}
};
#endif
enum
{
VLC_AVCODEC_MUTEX = 0,
VLC_GCRYPT_MUTEX,
VLC_XLIB_MUTEX,
VLC_MOSAIC_MUTEX,
#if defined(_WIN32)
VLC_MTA_MUTEX,
#endif
VLC_MAX_MUTEX
};
VLC_API void vlc_global_mutex(unsigned, bool);
#define vlc_global_lock( n ) vlc_global_mutex(n, true)
#define vlc_global_unlock( n ) vlc_global_mutex(n, false)
