#include "pthread.h"
#include "implement.h"
#include "sched.h"
int
sched_setscheduler (pid_t pid, int policy)
{
if (0 != pid)
{
int selfPid = (int) GetCurrentProcessId ();
if (pid != selfPid)
{
HANDLE h =
OpenProcess (PROCESS_SET_INFORMATION, PTW32_FALSE, (DWORD) pid);
if (NULL == h)
{
errno =
(GetLastError () ==
(0xFF & ERROR_ACCESS_DENIED)) ? EPERM : ESRCH;
return -1;
}
else
CloseHandle(h);
}
}
if (SCHED_OTHER != policy)
{
errno = ENOSYS;
return -1;
}
return SCHED_OTHER;
}
