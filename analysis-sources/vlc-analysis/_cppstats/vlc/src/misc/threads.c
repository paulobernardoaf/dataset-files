#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdatomic.h>
#include <vlc_common.h>
#include "libvlc.h"
struct vlc_suuint { union { unsigned int value; }; };
static_assert (sizeof (atomic_uint) <= sizeof (struct vlc_suuint),
"Size mismatch");
static_assert (alignof (atomic_uint) <= alignof (struct vlc_suuint),
"Alignment mismatch");
void vlc_global_mutex (unsigned n, bool acquire)
{
static vlc_mutex_t locks[] = {
VLC_STATIC_MUTEX,
VLC_STATIC_MUTEX,
VLC_STATIC_MUTEX,
VLC_STATIC_MUTEX,
#if defined(_WIN32)
VLC_STATIC_MUTEX, 
#endif
};
static_assert (VLC_MAX_MUTEX == ARRAY_SIZE(locks),
"Wrong number of global mutexes");
assert (n < ARRAY_SIZE(locks));
vlc_mutex_t *lock = locks + n;
if (acquire)
vlc_mutex_lock (lock);
else
vlc_mutex_unlock (lock);
}
#if defined (_WIN32) && (_WIN32_WINNT < _WIN32_WINNT_WIN8)
#undef LIBVLC_NEED_SLEEP
#endif
#if defined(_WIN32) || defined(__ANDROID__)
static void do_vlc_cancel_addr_clear(void *addr)
{
vlc_cancel_addr_clear(addr);
}
static void vlc_cancel_addr_prepare(atomic_uint *addr)
{
vlc_cancel_addr_set(addr);
vlc_cleanup_push(do_vlc_cancel_addr_clear, addr);
vlc_testcancel();
vlc_cleanup_pop();
}
static void vlc_cancel_addr_finish(atomic_uint *addr)
{
vlc_cancel_addr_clear(addr);
vlc_testcancel();
}
#else
#define vlc_cancel_addr_prepare(addr) ((void)0)
#define vlc_cancel_addr_finish(addr) ((void)0)
#endif
#if defined(LIBVLC_NEED_SLEEP)
void (vlc_tick_wait)(vlc_tick_t deadline)
{
atomic_uint value = ATOMIC_VAR_INIT(0);
vlc_cancel_addr_prepare(&value);
while (vlc_atomic_timedwait(&value, 0, deadline) == 0)
vlc_testcancel();
vlc_cancel_addr_finish(&value);
}
void (vlc_tick_sleep)(vlc_tick_t delay)
{
vlc_tick_wait(vlc_tick_now() + delay);
}
#endif
static void vlc_mutex_init_common(vlc_mutex_t *mtx, bool recursive)
{
atomic_init(&mtx->value, 0);
atomic_init(&mtx->recursion, recursive);
atomic_init(&mtx->owner, NULL);
}
void vlc_mutex_init(vlc_mutex_t *mtx)
{
vlc_mutex_init_common(mtx, false);
}
void vlc_mutex_init_recursive(vlc_mutex_t *mtx)
{
vlc_mutex_init_common(mtx, true);
}
static _Thread_local char thread_self[1];
#define THREAD_SELF ((const void *)thread_self)
bool vlc_mutex_held(const vlc_mutex_t *mtx)
{
#if defined(__clang__) && !defined(__apple_build_version__) && (__clang_major__ < 8)
vlc_mutex_t *tmp_mtx = (vlc_mutex_t *)mtx;
#else
const vlc_mutex_t *tmp_mtx = mtx;
#endif
return THREAD_SELF == atomic_load_explicit(&tmp_mtx->owner,
memory_order_relaxed);
}
void vlc_mutex_lock(vlc_mutex_t *mtx)
{
unsigned value;
if (vlc_mutex_trylock(mtx) == 0)
return;
int canc = vlc_savecancel(); 
while ((value = atomic_exchange_explicit(&mtx->value, 2,
memory_order_acquire)) != 0)
vlc_atomic_wait(&mtx->value, 2);
vlc_restorecancel(canc);
atomic_store_explicit(&mtx->owner, THREAD_SELF, memory_order_relaxed);
}
int vlc_mutex_trylock(vlc_mutex_t *mtx)
{
unsigned recursion = atomic_load_explicit(&mtx->recursion,
memory_order_relaxed);
if (unlikely(recursion) && vlc_mutex_held(mtx)) {
atomic_store_explicit(&mtx->recursion, recursion + 1,
memory_order_relaxed);
return 0;
} else
assert(!vlc_mutex_held(mtx));
unsigned value = 0;
if (atomic_compare_exchange_strong_explicit(&mtx->value, &value, 1,
memory_order_acquire,
memory_order_relaxed)) {
atomic_store_explicit(&mtx->owner, THREAD_SELF, memory_order_relaxed);
return 0;
}
return EBUSY;
}
void vlc_mutex_unlock(vlc_mutex_t *mtx)
{
assert(vlc_mutex_held(mtx));
unsigned recursion = atomic_load_explicit(&mtx->recursion,
memory_order_relaxed);
if (unlikely(recursion > 1)) {
atomic_store_explicit(&mtx->recursion, recursion - 1,
memory_order_relaxed);
return;
}
atomic_store_explicit(&mtx->owner, NULL, memory_order_relaxed);
switch (atomic_exchange_explicit(&mtx->value, 0, memory_order_release)) {
case 2:
vlc_atomic_notify_one(&mtx->value);
case 1:
break;
default:
vlc_assert_unreachable();
}
}
void vlc_cond_init(vlc_cond_t *cond)
{
cond->head = NULL;
vlc_mutex_init(&cond->lock);
}
struct vlc_cond_waiter {
struct vlc_cond_waiter **pprev, *next;
atomic_uint value;
vlc_cond_t *cond;
vlc_mutex_t *mutex;
};
static void vlc_cond_signal_waiter(struct vlc_cond_waiter *waiter)
{
waiter->pprev = &waiter->next;
waiter->next = NULL;
atomic_fetch_add_explicit(&waiter->value, 1, memory_order_relaxed);
vlc_atomic_notify_one(&waiter->value);
}
void vlc_cond_signal(vlc_cond_t *cond)
{
struct vlc_cond_waiter *waiter;
vlc_mutex_lock(&cond->lock);
waiter = cond->head;
if (waiter != NULL) {
struct vlc_cond_waiter *next = waiter->next;
struct vlc_cond_waiter **pprev = waiter->pprev;
*pprev = next;
if (next != NULL)
next->pprev = pprev;
vlc_cond_signal_waiter(waiter);
}
vlc_mutex_unlock(&cond->lock);
}
void vlc_cond_broadcast(vlc_cond_t *cond)
{
struct vlc_cond_waiter *waiter;
vlc_mutex_lock(&cond->lock);
waiter = cond->head;
cond->head = NULL;
while (waiter != NULL) {
struct vlc_cond_waiter *next = waiter->next;
vlc_cond_signal_waiter(waiter);
waiter = next;
}
vlc_mutex_unlock(&cond->lock);
}
static void vlc_cond_wait_prepare(struct vlc_cond_waiter *waiter,
vlc_cond_t *cond, vlc_mutex_t *mutex)
{
struct vlc_cond_waiter *next;
waiter->pprev = &cond->head;
atomic_init(&waiter->value, 0);
waiter->cond = cond;
waiter->mutex = mutex;
vlc_mutex_lock(&cond->lock);
next = cond->head;
cond->head = waiter;
waiter->next = next;
if (next != NULL)
next->pprev = &waiter->next;
vlc_mutex_unlock(&cond->lock);
vlc_cancel_addr_prepare(&waiter->value);
vlc_mutex_unlock(mutex);
}
static void vlc_cond_wait_finish(struct vlc_cond_waiter *waiter,
vlc_cond_t *cond, vlc_mutex_t *mutex)
{
struct vlc_cond_waiter *next;
vlc_mutex_lock(&cond->lock);
next = waiter->next;
*(waiter->pprev) = next;
if (next != NULL)
next->pprev = waiter->pprev;
vlc_mutex_unlock(&cond->lock);
vlc_mutex_lock(mutex);
vlc_cancel_addr_finish(&waiter->value);
}
static void vlc_cond_wait_cleanup(void *data)
{
struct vlc_cond_waiter *waiter = data;
vlc_cond_wait_finish(waiter, waiter->cond, waiter->mutex);
}
void vlc_cond_wait(vlc_cond_t *cond, vlc_mutex_t *mutex)
{
struct vlc_cond_waiter waiter;
vlc_cond_wait_prepare(&waiter, cond, mutex);
vlc_cleanup_push(vlc_cond_wait_cleanup, &waiter);
vlc_atomic_wait(&waiter.value, 0);
vlc_cleanup_pop();
vlc_cond_wait_cleanup(&waiter);
}
int vlc_cond_timedwait(vlc_cond_t *cond, vlc_mutex_t *mutex,
vlc_tick_t deadline)
{
struct vlc_cond_waiter waiter;
int ret;
vlc_cond_wait_prepare(&waiter, cond, mutex);
vlc_cleanup_push(vlc_cond_wait_cleanup, &waiter);
ret = vlc_atomic_timedwait(&waiter.value, 0, deadline);
vlc_cleanup_pop();
vlc_cond_wait_cleanup(&waiter);
return ret;
}
int vlc_cond_timedwait_daytime(vlc_cond_t *cond, vlc_mutex_t *mutex,
time_t deadline)
{
struct vlc_cond_waiter waiter;
int ret;
vlc_cond_wait_prepare(&waiter, cond, mutex);
vlc_cleanup_push(vlc_cond_wait_cleanup, &waiter);
ret = vlc_atomic_timedwait_daytime(&waiter.value, 0, deadline);
vlc_cleanup_pop();
vlc_cond_wait_cleanup(&waiter);
return ret;
}
#if defined(LIBVLC_NEED_RWLOCK)
#include <stdlib.h>
#include <limits.h>
#define READER_MASK LONG_MAX
#define WRITER_BIT LONG_MIN
void vlc_rwlock_init (vlc_rwlock_t *lock)
{
vlc_mutex_init (&lock->mutex);
vlc_cond_init (&lock->wait);
lock->state = 0;
}
void vlc_rwlock_destroy (vlc_rwlock_t *lock)
{
(void) lock;
}
void vlc_rwlock_rdlock (vlc_rwlock_t *lock)
{
vlc_mutex_lock (&lock->mutex);
while (lock->state < 0)
{
assert (lock->state == WRITER_BIT);
mutex_cleanup_push (&lock->mutex);
vlc_cond_wait (&lock->wait, &lock->mutex);
vlc_cleanup_pop ();
}
if (unlikely(lock->state >= READER_MASK))
abort (); 
lock->state++;
vlc_mutex_unlock (&lock->mutex);
}
void vlc_rwlock_wrlock (vlc_rwlock_t *lock)
{
vlc_mutex_lock (&lock->mutex);
while (lock->state != 0)
{
mutex_cleanup_push (&lock->mutex);
vlc_cond_wait (&lock->wait, &lock->mutex);
vlc_cleanup_pop ();
}
lock->state = WRITER_BIT;
vlc_mutex_unlock (&lock->mutex);
}
void vlc_rwlock_unlock (vlc_rwlock_t *lock)
{
vlc_mutex_lock (&lock->mutex);
if (lock->state < 0)
{ 
assert (lock->state == WRITER_BIT);
lock->state = 0;
vlc_cond_broadcast (&lock->wait);
}
else
{ 
assert (lock->state > 0);
if (--lock->state == 0)
vlc_cond_signal (&lock->wait);
}
vlc_mutex_unlock (&lock->mutex);
}
#endif 
void vlc_sem_init (vlc_sem_t *sem, unsigned value)
{
atomic_init(&sem->value, value);
}
int vlc_sem_post (vlc_sem_t *sem)
{
unsigned exp = atomic_load_explicit(&sem->value, memory_order_relaxed);
do
{
if (unlikely(exp == UINT_MAX))
return EOVERFLOW;
} while (!atomic_compare_exchange_weak_explicit(&sem->value, &exp, exp + 1,
memory_order_release,
memory_order_relaxed));
vlc_atomic_notify_one(&sem->value);
return 0;
}
void vlc_sem_wait (vlc_sem_t *sem)
{
unsigned exp = 1;
while (!atomic_compare_exchange_weak_explicit(&sem->value, &exp, exp - 1,
memory_order_acquire,
memory_order_relaxed))
{
if (likely(exp == 0))
{
vlc_atomic_wait(&sem->value, 0);
exp = 1;
}
}
}
int vlc_sem_timedwait(vlc_sem_t *sem, vlc_tick_t deadline)
{
unsigned exp = 1;
while (!atomic_compare_exchange_weak_explicit(&sem->value, &exp, exp - 1,
memory_order_acquire,
memory_order_relaxed))
{
if (likely(exp == 0))
{
int ret = vlc_atomic_timedwait(&sem->value, 0, deadline);
if (ret)
return ret;
exp = 1;
}
}
return 0;
}
enum { VLC_ONCE_UNDONE, VLC_ONCE_DOING, VLC_ONCE_CONTEND, VLC_ONCE_DONE };
static_assert (VLC_ONCE_DONE == 3, "Check vlc_once in header file");
void (vlc_once)(vlc_once_t *restrict once, void (*cb)(void))
{
unsigned int value = VLC_ONCE_UNDONE;
if (atomic_compare_exchange_strong_explicit(&once->value, &value,
VLC_ONCE_DOING,
memory_order_acquire,
memory_order_acquire)) {
cb();
if (atomic_exchange_explicit(&once->value, VLC_ONCE_DONE,
memory_order_release) == VLC_ONCE_CONTEND)
vlc_atomic_notify_all(&once->value);
return;
}
assert(value >= VLC_ONCE_DOING);
if (unlikely(value == VLC_ONCE_DOING)
&& atomic_compare_exchange_strong_explicit(&once->value, &value,
VLC_ONCE_CONTEND,
memory_order_acquire,
memory_order_acquire))
value = VLC_ONCE_CONTEND;
assert(value >= VLC_ONCE_CONTEND);
while (unlikely(value != VLC_ONCE_DONE)) {
vlc_atomic_wait(&once->value, VLC_ONCE_CONTEND);
value = atomic_load_explicit(&once->value, memory_order_acquire);
}
}
