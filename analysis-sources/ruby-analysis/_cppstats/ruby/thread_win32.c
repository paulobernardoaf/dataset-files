#if defined(THREAD_SYSTEM_DEPENDENT_IMPLEMENTATION)
#include <process.h>
#define TIME_QUANTUM_USEC (10 * 1000)
#define RB_CONDATTR_CLOCK_MONOTONIC 1 
#undef Sleep
#define native_thread_yield() Sleep(0)
#define unregister_ubf_list(th)
#define ubf_wakeup_all_threads() do {} while (0)
#define ubf_threads_empty() (1)
#define ubf_timer_disarm() do {} while (0)
#define ubf_list_atfork() do {} while (0)
static volatile DWORD ruby_native_thread_key = TLS_OUT_OF_INDEXES;
static int w32_wait_events(HANDLE *events, int count, DWORD timeout, rb_thread_t *th);
void rb_native_mutex_lock(rb_nativethread_lock_t *lock);
void rb_native_mutex_unlock(rb_nativethread_lock_t *lock);
static void
w32_error(const char *func)
{
LPVOID lpMsgBuf;
DWORD err = GetLastError();
if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
FORMAT_MESSAGE_FROM_SYSTEM |
FORMAT_MESSAGE_IGNORE_INSERTS,
NULL,
err,
MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
(LPTSTR) & lpMsgBuf, 0, NULL) == 0)
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
FORMAT_MESSAGE_FROM_SYSTEM |
FORMAT_MESSAGE_IGNORE_INSERTS,
NULL,
err,
MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
(LPTSTR) & lpMsgBuf, 0, NULL);
rb_bug("%s: %s", func, (char*)lpMsgBuf);
}
static int
w32_mutex_lock(HANDLE lock)
{
DWORD result;
while (1) {
thread_debug("rb_native_mutex_lock: %p\n", lock);
result = w32_wait_events(&lock, 1, INFINITE, 0);
switch (result) {
case WAIT_OBJECT_0:
thread_debug("acquire mutex: %p\n", lock);
return 0;
case WAIT_OBJECT_0 + 1:
errno = EINTR;
thread_debug("acquire mutex interrupted: %p\n", lock);
return 0;
case WAIT_TIMEOUT:
thread_debug("timeout mutex: %p\n", lock);
break;
case WAIT_ABANDONED:
rb_bug("win32_mutex_lock: WAIT_ABANDONED");
break;
default:
rb_bug("win32_mutex_lock: unknown result (%ld)", result);
break;
}
}
return 0;
}
static HANDLE
w32_mutex_create(void)
{
HANDLE lock = CreateMutex(NULL, FALSE, NULL);
if (lock == NULL) {
w32_error("rb_native_mutex_initialize");
}
return lock;
}
#define GVL_DEBUG 0
static void
gvl_acquire(rb_vm_t *vm, rb_thread_t *th)
{
w32_mutex_lock(vm->gvl.lock);
if (GVL_DEBUG) fprintf(stderr, "gvl acquire (%p): acquire\n", th);
}
static void
gvl_release(rb_vm_t *vm)
{
ReleaseMutex(vm->gvl.lock);
}
static void
gvl_yield(rb_vm_t *vm, rb_thread_t *th)
{
gvl_release(th->vm);
native_thread_yield();
gvl_acquire(vm, th);
}
static void
gvl_init(rb_vm_t *vm)
{
if (GVL_DEBUG) fprintf(stderr, "gvl init\n");
vm->gvl.lock = w32_mutex_create();
}
static void
gvl_destroy(rb_vm_t *vm)
{
if (GVL_DEBUG) fprintf(stderr, "gvl destroy\n");
CloseHandle(vm->gvl.lock);
}
static rb_thread_t *
ruby_thread_from_native(void)
{
return TlsGetValue(ruby_native_thread_key);
}
static int
ruby_thread_set_native(rb_thread_t *th)
{
return TlsSetValue(ruby_native_thread_key, th);
}
void
Init_native_thread(rb_thread_t *th)
{
ruby_native_thread_key = TlsAlloc();
ruby_thread_set_native(th);
DuplicateHandle(GetCurrentProcess(),
GetCurrentThread(),
GetCurrentProcess(),
&th->thread_id, 0, FALSE, DUPLICATE_SAME_ACCESS);
th->native_thread_data.interrupt_event = CreateEvent(0, TRUE, FALSE, 0);
thread_debug("initial thread (th: %p, thid: %p, event: %p)\n",
th, GET_THREAD()->thread_id,
th->native_thread_data.interrupt_event);
}
static int
w32_wait_events(HANDLE *events, int count, DWORD timeout, rb_thread_t *th)
{
HANDLE *targets = events;
HANDLE intr;
const int initcount = count;
DWORD ret;
thread_debug(" w32_wait_events events:%p, count:%d, timeout:%ld, th:%p\n",
events, count, timeout, th);
if (th && (intr = th->native_thread_data.interrupt_event)) {
if (ResetEvent(intr) && (!RUBY_VM_INTERRUPTED(th->ec) || SetEvent(intr))) {
targets = ALLOCA_N(HANDLE, count + 1);
memcpy(targets, events, sizeof(HANDLE) * count);
targets[count++] = intr;
thread_debug(" * handle: %p (count: %d, intr)\n", intr, count);
}
else if (intr == th->native_thread_data.interrupt_event) {
w32_error("w32_wait_events");
}
}
thread_debug(" WaitForMultipleObjects start (count: %d)\n", count);
ret = WaitForMultipleObjects(count, targets, FALSE, timeout);
thread_debug(" WaitForMultipleObjects end (ret: %lu)\n", ret);
if (ret == (DWORD)(WAIT_OBJECT_0 + initcount) && th) {
errno = EINTR;
}
if (ret == WAIT_FAILED && THREAD_DEBUG) {
int i;
DWORD dmy;
for (i = 0; i < count; i++) {
thread_debug(" * error handle %d - %s\n", i,
GetHandleInformation(targets[i], &dmy) ? "OK" : "NG");
}
}
return ret;
}
static void ubf_handle(void *ptr);
#define ubf_select ubf_handle
int
rb_w32_wait_events_blocking(HANDLE *events, int num, DWORD timeout)
{
return w32_wait_events(events, num, timeout, ruby_thread_from_native());
}
int
rb_w32_wait_events(HANDLE *events, int num, DWORD timeout)
{
int ret;
rb_thread_t *th = GET_THREAD();
BLOCKING_REGION(th, ret = rb_w32_wait_events_blocking(events, num, timeout),
ubf_handle, ruby_thread_from_native(), FALSE);
return ret;
}
static void
w32_close_handle(HANDLE handle)
{
if (CloseHandle(handle) == 0) {
w32_error("w32_close_handle");
}
}
static void
w32_resume_thread(HANDLE handle)
{
if (ResumeThread(handle) == (DWORD)-1) {
w32_error("w32_resume_thread");
}
}
#if defined(_MSC_VER)
#define HAVE__BEGINTHREADEX 1
#else
#undef HAVE__BEGINTHREADEX
#endif
#if defined(HAVE__BEGINTHREADEX)
#define start_thread (HANDLE)_beginthreadex
#define thread_errno errno
typedef unsigned long (__stdcall *w32_thread_start_func)(void*);
#else
#define start_thread CreateThread
#define thread_errno rb_w32_map_errno(GetLastError())
typedef LPTHREAD_START_ROUTINE w32_thread_start_func;
#endif
static HANDLE
w32_create_thread(DWORD stack_size, w32_thread_start_func func, void *val)
{
return start_thread(0, stack_size, func, val, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, 0);
}
int
rb_w32_sleep(unsigned long msec)
{
return w32_wait_events(0, 0, msec, ruby_thread_from_native());
}
int WINAPI
rb_w32_Sleep(unsigned long msec)
{
int ret;
rb_thread_t *th = GET_THREAD();
BLOCKING_REGION(th, ret = rb_w32_sleep(msec),
ubf_handle, ruby_thread_from_native(), FALSE);
return ret;
}
static DWORD
hrtime2msec(rb_hrtime_t hrt)
{
return (DWORD)hrt / (DWORD)RB_HRTIME_PER_MSEC;
}
static void
native_sleep(rb_thread_t *th, rb_hrtime_t *rel)
{
const volatile DWORD msec = rel ? hrtime2msec(*rel) : INFINITE;
GVL_UNLOCK_BEGIN(th);
{
DWORD ret;
rb_native_mutex_lock(&th->interrupt_lock);
th->unblock.func = ubf_handle;
th->unblock.arg = th;
rb_native_mutex_unlock(&th->interrupt_lock);
if (RUBY_VM_INTERRUPTED(th->ec)) {
}
else {
thread_debug("native_sleep start (%lu)\n", msec);
ret = w32_wait_events(0, 0, msec, th);
thread_debug("native_sleep done (%lu)\n", ret);
}
rb_native_mutex_lock(&th->interrupt_lock);
th->unblock.func = 0;
th->unblock.arg = 0;
rb_native_mutex_unlock(&th->interrupt_lock);
}
GVL_UNLOCK_END(th);
}
void
rb_native_mutex_lock(rb_nativethread_lock_t *lock)
{
#if USE_WIN32_MUTEX
w32_mutex_lock(lock->mutex);
#else
EnterCriticalSection(&lock->crit);
#endif
}
void
rb_native_mutex_unlock(rb_nativethread_lock_t *lock)
{
#if USE_WIN32_MUTEX
thread_debug("release mutex: %p\n", lock->mutex);
ReleaseMutex(lock->mutex);
#else
LeaveCriticalSection(&lock->crit);
#endif
}
static int
native_mutex_trylock(rb_nativethread_lock_t *lock)
{
#if USE_WIN32_MUTEX
int result;
thread_debug("native_mutex_trylock: %p\n", lock->mutex);
result = w32_wait_events(&lock->mutex, 1, 1, 0);
thread_debug("native_mutex_trylock result: %d\n", result);
switch (result) {
case WAIT_OBJECT_0:
return 0;
case WAIT_TIMEOUT:
return EBUSY;
}
return EINVAL;
#else
return TryEnterCriticalSection(&lock->crit) == 0;
#endif
}
void
rb_native_mutex_initialize(rb_nativethread_lock_t *lock)
{
#if USE_WIN32_MUTEX
lock->mutex = w32_mutex_create();
#else
InitializeCriticalSection(&lock->crit);
#endif
}
void
rb_native_mutex_destroy(rb_nativethread_lock_t *lock)
{
#if USE_WIN32_MUTEX
w32_close_handle(lock->mutex);
#else
DeleteCriticalSection(&lock->crit);
#endif
}
struct cond_event_entry {
struct cond_event_entry* next;
struct cond_event_entry* prev;
HANDLE event;
};
void
rb_native_cond_signal(rb_nativethread_cond_t *cond)
{
struct cond_event_entry *e = cond->next;
struct cond_event_entry *head = (struct cond_event_entry*)cond;
if (e != head) {
struct cond_event_entry *next = e->next;
struct cond_event_entry *prev = e->prev;
prev->next = next;
next->prev = prev;
e->next = e->prev = e;
SetEvent(e->event);
}
}
void
rb_native_cond_broadcast(rb_nativethread_cond_t *cond)
{
struct cond_event_entry *e = cond->next;
struct cond_event_entry *head = (struct cond_event_entry*)cond;
while (e != head) {
struct cond_event_entry *next = e->next;
struct cond_event_entry *prev = e->prev;
SetEvent(e->event);
prev->next = next;
next->prev = prev;
e->next = e->prev = e;
e = next;
}
}
static int
native_cond_timedwait_ms(rb_nativethread_cond_t *cond, rb_nativethread_lock_t *mutex, unsigned long msec)
{
DWORD r;
struct cond_event_entry entry;
struct cond_event_entry *head = (struct cond_event_entry*)cond;
entry.event = CreateEvent(0, FALSE, FALSE, 0);
entry.next = head;
entry.prev = head->prev;
head->prev->next = &entry;
head->prev = &entry;
rb_native_mutex_unlock(mutex);
{
r = WaitForSingleObject(entry.event, msec);
if ((r != WAIT_OBJECT_0) && (r != WAIT_TIMEOUT)) {
rb_bug("rb_native_cond_wait: WaitForSingleObject returns %lu", r);
}
}
rb_native_mutex_lock(mutex);
entry.prev->next = entry.next;
entry.next->prev = entry.prev;
w32_close_handle(entry.event);
return (r == WAIT_OBJECT_0) ? 0 : ETIMEDOUT;
}
void
rb_native_cond_wait(rb_nativethread_cond_t *cond, rb_nativethread_lock_t *mutex)
{
native_cond_timedwait_ms(cond, mutex, INFINITE);
}
#if 0
static unsigned long
abs_timespec_to_timeout_ms(const struct timespec *ts)
{
struct timeval tv;
struct timeval now;
gettimeofday(&now, NULL);
tv.tv_sec = ts->tv_sec;
tv.tv_usec = ts->tv_nsec / 1000;
if (!rb_w32_time_subtract(&tv, &now))
return 0;
return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
static int
native_cond_timedwait(rb_nativethread_cond_t *cond, rb_nativethread_lock_t *mutex, const struct timespec *ts)
{
unsigned long timeout_ms;
timeout_ms = abs_timespec_to_timeout_ms(ts);
if (!timeout_ms)
return ETIMEDOUT;
return native_cond_timedwait_ms(cond, mutex, timeout_ms);
}
static struct timespec
native_cond_timeout(rb_nativethread_cond_t *cond, struct timespec timeout_rel)
{
int ret;
struct timeval tv;
struct timespec timeout;
struct timespec now;
ret = gettimeofday(&tv, 0);
if (ret != 0)
rb_sys_fail(0);
now.tv_sec = tv.tv_sec;
now.tv_nsec = tv.tv_usec * 1000;
timeout.tv_sec = now.tv_sec;
timeout.tv_nsec = now.tv_nsec;
timeout.tv_sec += timeout_rel.tv_sec;
timeout.tv_nsec += timeout_rel.tv_nsec;
if (timeout.tv_nsec >= 1000*1000*1000) {
timeout.tv_sec++;
timeout.tv_nsec -= 1000*1000*1000;
}
if (timeout.tv_sec < now.tv_sec)
timeout.tv_sec = TIMET_MAX;
return timeout;
}
#endif
void
rb_native_cond_initialize(rb_nativethread_cond_t *cond)
{
cond->next = (struct cond_event_entry *)cond;
cond->prev = (struct cond_event_entry *)cond;
}
void
rb_native_cond_destroy(rb_nativethread_cond_t *cond)
{
}
void
ruby_init_stack(volatile VALUE *addr)
{
}
#define CHECK_ERR(expr) {if (!(expr)) {rb_bug("err: %lu - %s", GetLastError(), #expr);}}
static void
native_thread_init_stack(rb_thread_t *th)
{
MEMORY_BASIC_INFORMATION mi;
char *base, *end;
DWORD size, space;
CHECK_ERR(VirtualQuery(&mi, &mi, sizeof(mi)));
base = mi.AllocationBase;
end = mi.BaseAddress;
end += mi.RegionSize;
size = end - base;
space = size / 5;
if (space > 1024*1024) space = 1024*1024;
th->ec->machine.stack_start = (VALUE *)end - 1;
th->ec->machine.stack_maxsize = size - space;
}
#if !defined(InterlockedExchangePointer)
#define InterlockedExchangePointer(t, v) (void *)InterlockedExchange((long *)(t), (long)(v))
#endif
static void
native_thread_destroy(rb_thread_t *th)
{
HANDLE intr = InterlockedExchangePointer(&th->native_thread_data.interrupt_event, 0);
thread_debug("close handle - intr: %p, thid: %p\n", intr, th->thread_id);
w32_close_handle(intr);
}
static unsigned long __stdcall
thread_start_func_1(void *th_ptr)
{
rb_thread_t *th = th_ptr;
volatile HANDLE thread_id = th->thread_id;
native_thread_init_stack(th);
th->native_thread_data.interrupt_event = CreateEvent(0, TRUE, FALSE, 0);
thread_debug("thread created (th: %p, thid: %p, event: %p)\n", th,
th->thread_id, th->native_thread_data.interrupt_event);
thread_start_func_2(th, th->ec->machine.stack_start);
w32_close_handle(thread_id);
thread_debug("thread deleted (th: %p)\n", th);
return 0;
}
static int
native_thread_create(rb_thread_t *th)
{
const size_t stack_size = th->vm->default_params.thread_machine_stack_size + th->vm->default_params.thread_vm_stack_size;
th->thread_id = w32_create_thread(stack_size, thread_start_func_1, th);
if ((th->thread_id) == 0) {
return thread_errno;
}
w32_resume_thread(th->thread_id);
if (THREAD_DEBUG) {
Sleep(0);
thread_debug("create: (th: %p, thid: %p, intr: %p), stack size: %"PRIuSIZE"\n",
th, th->thread_id,
th->native_thread_data.interrupt_event, stack_size);
}
return 0;
}
static void
native_thread_join(HANDLE th)
{
w32_wait_events(&th, 1, INFINITE, 0);
}
#if USE_NATIVE_THREAD_PRIORITY
static void
native_thread_apply_priority(rb_thread_t *th)
{
int priority = th->priority;
if (th->priority > 0) {
priority = THREAD_PRIORITY_ABOVE_NORMAL;
}
else if (th->priority < 0) {
priority = THREAD_PRIORITY_BELOW_NORMAL;
}
else {
priority = THREAD_PRIORITY_NORMAL;
}
SetThreadPriority(th->thread_id, priority);
}
#endif 
int rb_w32_select_with_thread(int, fd_set *, fd_set *, fd_set *, struct timeval *, void *); 
static int
native_fd_select(int n, rb_fdset_t *readfds, rb_fdset_t *writefds, rb_fdset_t *exceptfds, struct timeval *timeout, rb_thread_t *th)
{
fd_set *r = NULL, *w = NULL, *e = NULL;
if (readfds) {
rb_fd_resize(n - 1, readfds);
r = rb_fd_ptr(readfds);
}
if (writefds) {
rb_fd_resize(n - 1, writefds);
w = rb_fd_ptr(writefds);
}
if (exceptfds) {
rb_fd_resize(n - 1, exceptfds);
e = rb_fd_ptr(exceptfds);
}
return rb_w32_select_with_thread(n, r, w, e, timeout, th);
}
int
rb_w32_check_interrupt(rb_thread_t *th)
{
return w32_wait_events(0, 0, 0, th);
}
static void
ubf_handle(void *ptr)
{
rb_thread_t *th = (rb_thread_t *)ptr;
thread_debug("ubf_handle: %p\n", th);
if (!SetEvent(th->native_thread_data.interrupt_event)) {
w32_error("ubf_handle");
}
}
int rb_w32_set_thread_description(HANDLE th, const WCHAR *name);
int rb_w32_set_thread_description_str(HANDLE th, VALUE name);
#define native_set_another_thread_name rb_w32_set_thread_description_str
static struct {
HANDLE id;
HANDLE lock;
} timer_thread;
#define TIMER_THREAD_CREATED_P() (timer_thread.id != 0)
static unsigned long __stdcall
timer_thread_func(void *dummy)
{
rb_vm_t *vm = GET_VM();
thread_debug("timer_thread\n");
rb_w32_set_thread_description(GetCurrentThread(), L"ruby-timer-thread");
while (WaitForSingleObject(timer_thread.lock, TIME_QUANTUM_USEC/1000) ==
WAIT_TIMEOUT) {
timer_thread_function();
ruby_sigchld_handler(vm); 
rb_threadptr_check_signal(vm->main_thread);
}
thread_debug("timer killed\n");
return 0;
}
void
rb_thread_wakeup_timer_thread(int sig)
{
}
static VALUE
rb_thread_start_unblock_thread(void)
{
return Qfalse; 
}
static void
rb_thread_create_timer_thread(void)
{
if (timer_thread.id == 0) {
if (!timer_thread.lock) {
timer_thread.lock = CreateEvent(0, TRUE, FALSE, 0);
}
timer_thread.id = w32_create_thread(1024 + (THREAD_DEBUG ? BUFSIZ : 0),
timer_thread_func, 0);
w32_resume_thread(timer_thread.id);
}
}
static int
native_stop_timer_thread(void)
{
int stopped = --system_working <= 0;
if (stopped) {
SetEvent(timer_thread.lock);
native_thread_join(timer_thread.id);
CloseHandle(timer_thread.lock);
timer_thread.lock = 0;
}
return stopped;
}
static void
native_reset_timer_thread(void)
{
if (timer_thread.id) {
CloseHandle(timer_thread.id);
timer_thread.id = 0;
}
}
int
ruby_stack_overflowed_p(const rb_thread_t *th, const void *addr)
{
return rb_ec_raised_p(th->ec, RAISED_STACKOVERFLOW);
}
#if defined(__MINGW32__)
LONG WINAPI
rb_w32_stack_overflow_handler(struct _EXCEPTION_POINTERS *exception)
{
if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
rb_ec_raised_set(GET_EC(), RAISED_STACKOVERFLOW);
raise(SIGSEGV);
}
return EXCEPTION_CONTINUE_SEARCH;
}
#endif
#if defined(RUBY_ALLOCA_CHKSTK)
void
ruby_alloca_chkstk(size_t len, void *sp)
{
if (ruby_stack_length(NULL) * sizeof(VALUE) >= len) {
rb_execution_context_t *ec = GET_EC();
if (!rb_ec_raised_p(ec, RAISED_STACKOVERFLOW)) {
rb_ec_raised_set(ec, RAISED_STACKOVERFLOW);
rb_exc_raise(sysstack_error);
}
}
}
#endif
int
rb_reserved_fd_p(int fd)
{
return 0;
}
int
rb_sigwait_fd_get(rb_thread_t *th)
{
return -1; 
}
NORETURN(void rb_sigwait_fd_put(rb_thread_t *, int));
void
rb_sigwait_fd_put(rb_thread_t *th, int fd)
{
rb_bug("not implemented, should not be called");
}
NORETURN(void rb_sigwait_sleep(const rb_thread_t *, int, const rb_hrtime_t *));
void
rb_sigwait_sleep(const rb_thread_t *th, int fd, const rb_hrtime_t *rel)
{
rb_bug("not implemented, should not be called");
}
rb_nativethread_id_t
rb_nativethread_self(void)
{
return GetCurrentThread();
}
static void
native_set_thread_name(rb_thread_t *th)
{
}
#if USE_MJIT
static unsigned long __stdcall
mjit_worker(void *arg)
{
void (*worker_func)(void) = arg;
rb_w32_set_thread_description(GetCurrentThread(), L"ruby-mjitworker");
worker_func();
return 0;
}
int
rb_thread_create_mjit_thread(void (*worker_func)(void))
{
size_t stack_size = 4 * 1024; 
HANDLE thread_id = w32_create_thread(stack_size, mjit_worker, worker_func);
if (thread_id == 0) {
return FALSE;
}
w32_resume_thread(thread_id);
return TRUE;
}
#endif
#endif 
