










#if !defined(RUBY_THREAD_WIN32_H)
#define RUBY_THREAD_WIN32_H

#if defined(__CYGWIN__)
#undef _WIN32
#endif

WINBASEAPI BOOL WINAPI
TryEnterCriticalSection(IN OUT LPCRITICAL_SECTION lpCriticalSection);

typedef struct rb_thread_cond_struct {
struct cond_event_entry *next;
struct cond_event_entry *prev;
} rb_nativethread_cond_t;

typedef struct native_thread_data_struct {
HANDLE interrupt_event;
} native_thread_data_t;

typedef struct rb_global_vm_lock_struct {
HANDLE lock;
} rb_global_vm_lock_t;

#endif 

