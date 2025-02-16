















#pragma once









#include "c99defs.h"

#if defined(_MSC_VER)
#include "../../deps/w32-pthreads/pthread.h"
#else
#include <errno.h>
#include <pthread.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_WIN32)
#include "threading-windows.h"
#else
#include "threading-posix.h"
#endif


static inline void pthread_mutex_init_value(pthread_mutex_t *mutex)
{
pthread_mutex_t init_val = PTHREAD_MUTEX_INITIALIZER;
if (!mutex)
return;

*mutex = init_val;
}

enum os_event_type {
OS_EVENT_TYPE_AUTO,
OS_EVENT_TYPE_MANUAL,
};

struct os_event_data;
struct os_sem_data;
typedef struct os_event_data os_event_t;
typedef struct os_sem_data os_sem_t;

EXPORT int os_event_init(os_event_t **event, enum os_event_type type);
EXPORT void os_event_destroy(os_event_t *event);
EXPORT int os_event_wait(os_event_t *event);
EXPORT int os_event_timedwait(os_event_t *event, unsigned long milliseconds);
EXPORT int os_event_try(os_event_t *event);
EXPORT int os_event_signal(os_event_t *event);
EXPORT void os_event_reset(os_event_t *event);

EXPORT int os_sem_init(os_sem_t **sem, int value);
EXPORT void os_sem_destroy(os_sem_t *sem);
EXPORT int os_sem_post(os_sem_t *sem);
EXPORT int os_sem_wait(os_sem_t *sem);

EXPORT void os_set_thread_name(const char *name);

#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

#if defined(__cplusplus)
}
#endif
