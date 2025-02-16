








#include "../../git-compat-util.h"
#include "pthread.h"

#include <errno.h>
#include <limits.h>

static unsigned __stdcall win32_start_routine(void *arg)
{
pthread_t *thread = arg;
thread->tid = GetCurrentThreadId();
thread->arg = thread->start_routine(thread->arg);
return 0;
}

int pthread_create(pthread_t *thread, const void *unused,
void *(*start_routine)(void*), void *arg)
{
thread->arg = arg;
thread->start_routine = start_routine;
thread->handle = (HANDLE)
_beginthreadex(NULL, 0, win32_start_routine, thread, 0, NULL);

if (!thread->handle)
return errno;
else
return 0;
}

int win32_pthread_join(pthread_t *thread, void **value_ptr)
{
DWORD result = WaitForSingleObject(thread->handle, INFINITE);
switch (result) {
case WAIT_OBJECT_0:
if (value_ptr)
*value_ptr = thread->arg;
return 0;
case WAIT_ABANDONED:
return EINVAL;
default:
return err_win_to_posix(GetLastError());
}
}

pthread_t pthread_self(void)
{
pthread_t t = { NULL };
t.tid = GetCurrentThreadId();
return t;
}
