










#if !defined(RUBY_THREAD_NATIVE_H)
#define RUBY_THREAD_NATIVE_H 1












#if defined(_WIN32)
#include <windows.h>
typedef HANDLE rb_nativethread_id_t;

typedef union rb_thread_lock_union {
HANDLE mutex;
CRITICAL_SECTION crit;
} rb_nativethread_lock_t;

#elif defined(HAVE_PTHREAD_H)
#include <pthread.h>
typedef pthread_t rb_nativethread_id_t;
typedef pthread_mutex_t rb_nativethread_lock_t;

#else
#error "unsupported thread type"

#endif

RUBY_SYMBOL_EXPORT_BEGIN

rb_nativethread_id_t rb_nativethread_self();

void rb_nativethread_lock_initialize(rb_nativethread_lock_t *lock);
void rb_nativethread_lock_destroy(rb_nativethread_lock_t *lock);
void rb_nativethread_lock_lock(rb_nativethread_lock_t *lock);
void rb_nativethread_lock_unlock(rb_nativethread_lock_t *lock);

RUBY_SYMBOL_EXPORT_END

#endif
