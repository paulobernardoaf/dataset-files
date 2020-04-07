




































#include "pthread.h"
#include "implement.h"





#if !defined(WINCE)
#include <signal.h>
#endif


int
pthread_detach (pthread_t thread)
























{
int result;
BOOL destroyIt = PTW32_FALSE;
ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;
ptw32_mcs_local_node_t node;

ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);

if (NULL == tp
|| thread.x != tp->ptHandle.x)
{
result = ESRCH;
}
else if (PTHREAD_CREATE_DETACHED == tp->detachState)
{
result = EINVAL;
}
else
{
ptw32_mcs_local_node_t stateLock;





result = 0;

ptw32_mcs_lock_acquire (&tp->stateLock, &stateLock);
if (tp->state != PThreadStateLast)
{
tp->detachState = PTHREAD_CREATE_DETACHED;
}
else if (tp->detachState != PTHREAD_CREATE_DETACHED)
{



destroyIt = PTW32_TRUE;
}
ptw32_mcs_lock_release (&stateLock);
}

ptw32_mcs_lock_release(&node);

if (result == 0)
{


if (destroyIt)
{



(void) WaitForSingleObject(tp->threadH, INFINITE);
ptw32_threadDestroy (thread);
}
}

return (result);

} 
