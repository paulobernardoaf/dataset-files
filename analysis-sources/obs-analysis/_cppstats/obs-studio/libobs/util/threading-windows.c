#include "bmem.h"
#include "threading.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__MINGW32__)
#include <excpt.h>
#if !defined(TRYLEVEL_NONE)
#if !defined(__MINGW64__)
#define NO_SEH_MINGW
#endif
#if !defined(__try)
#define __try
#endif
#if !defined(__except)
#define __except (x) if (0)
#endif
#endif
#endif
int os_event_init(os_event_t **event, enum os_event_type type)
{
HANDLE handle;
handle = CreateEvent(NULL, (type == OS_EVENT_TYPE_MANUAL), FALSE, NULL);
if (!handle)
return -1;
*event = (os_event_t *)handle;
return 0;
}
void os_event_destroy(os_event_t *event)
{
if (event)
CloseHandle((HANDLE)event);
}
int os_event_wait(os_event_t *event)
{
DWORD code;
if (!event)
return EINVAL;
code = WaitForSingleObject((HANDLE)event, INFINITE);
if (code != WAIT_OBJECT_0)
return EINVAL;
return 0;
}
int os_event_timedwait(os_event_t *event, unsigned long milliseconds)
{
DWORD code;
if (!event)
return EINVAL;
code = WaitForSingleObject((HANDLE)event, milliseconds);
if (code == WAIT_TIMEOUT)
return ETIMEDOUT;
else if (code != WAIT_OBJECT_0)
return EINVAL;
return 0;
}
int os_event_try(os_event_t *event)
{
DWORD code;
if (!event)
return EINVAL;
code = WaitForSingleObject((HANDLE)event, 0);
if (code == WAIT_TIMEOUT)
return EAGAIN;
else if (code != WAIT_OBJECT_0)
return EINVAL;
return 0;
}
int os_event_signal(os_event_t *event)
{
if (!event)
return EINVAL;
if (!SetEvent((HANDLE)event))
return EINVAL;
return 0;
}
void os_event_reset(os_event_t *event)
{
if (!event)
return;
ResetEvent((HANDLE)event);
}
int os_sem_init(os_sem_t **sem, int value)
{
HANDLE handle = CreateSemaphore(NULL, (LONG)value, 0x7FFFFFFF, NULL);
if (!handle)
return -1;
*sem = (os_sem_t *)handle;
return 0;
}
void os_sem_destroy(os_sem_t *sem)
{
if (sem)
CloseHandle((HANDLE)sem);
}
int os_sem_post(os_sem_t *sem)
{
if (!sem)
return -1;
return ReleaseSemaphore((HANDLE)sem, 1, NULL) ? 0 : -1;
}
int os_sem_wait(os_sem_t *sem)
{
DWORD ret;
if (!sem)
return -1;
ret = WaitForSingleObject((HANDLE)sem, INFINITE);
return (ret == WAIT_OBJECT_0) ? 0 : -1;
}
#define VC_EXCEPTION 0x406D1388
#pragma pack(push, 8)
struct vs_threadname_info {
DWORD type; 
const char *name;
DWORD thread_id;
DWORD flags;
};
#pragma pack(pop)
#define THREADNAME_INFO_SIZE (sizeof(struct vs_threadname_info) / sizeof(ULONG_PTR))
void os_set_thread_name(const char *name)
{
#if defined(__MINGW32__)
UNUSED_PARAMETER(name);
#else
struct vs_threadname_info info;
info.type = 0x1000;
info.name = name;
info.thread_id = GetCurrentThreadId();
info.flags = 0;
#if defined(NO_SEH_MINGW)
__try1(EXCEPTION_EXECUTE_HANDLER)
{
#else
__try {
#endif
RaiseException(VC_EXCEPTION, 0, THREADNAME_INFO_SIZE,
(ULONG_PTR *)&info);
#if defined(NO_SEH_MINGW)
}
__except1{
#else
} __except (EXCEPTION_EXECUTE_HANDLER) {
#endif
}
#endif
}
