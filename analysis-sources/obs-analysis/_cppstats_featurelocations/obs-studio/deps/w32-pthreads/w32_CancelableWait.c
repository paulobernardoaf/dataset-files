



































#include "pthread.h"
#include "implement.h"


static INLINE int
ptw32_cancelable_wait (HANDLE waitHandle, DWORD timeout)













{
int result;
pthread_t self;
ptw32_thread_t * sp;
HANDLE handles[2];
DWORD nHandles = 1;
DWORD status;

handles[0] = waitHandle;

self = pthread_self();
sp = (ptw32_thread_t *) self.p;

if (sp != NULL)
{



if (sp->cancelState == PTHREAD_CANCEL_ENABLE)
{

if ((handles[1] = sp->cancelEvent) != NULL)
{
nHandles++;
}
}
}
else
{
handles[1] = NULL;
}

status = WaitForMultipleObjects (nHandles, handles, PTW32_FALSE, timeout);

switch (status - WAIT_OBJECT_0)
{
case 0:







result = 0;
break;

case 1:





ResetEvent (handles[1]);

if (sp != NULL)
{
ptw32_mcs_local_node_t stateLock;




ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
if (sp->state < PThreadStateCanceling)
{
sp->state = PThreadStateCanceling;
sp->cancelState = PTHREAD_CANCEL_DISABLE;
ptw32_mcs_lock_release (&stateLock);
ptw32_throw (PTW32_EPS_CANCEL);


}
ptw32_mcs_lock_release (&stateLock);
}


result = EINVAL;
break;

default:
if (status == WAIT_TIMEOUT)
{
result = ETIMEDOUT;
}
else
{
result = EINVAL;
}
break;
}

return (result);

} 

int
pthreadCancelableWait (HANDLE waitHandle)
{
return (ptw32_cancelable_wait (waitHandle, INFINITE));
}

int
pthreadCancelableTimedWait (HANDLE waitHandle, DWORD timeout)
{
return (ptw32_cancelable_wait (waitHandle, timeout));
}
