#include "pthread.h"
#include "implement.h"
int
pthread_setcanceltype (int type, int *oldtype)
{
ptw32_mcs_local_node_t stateLock;
int result = 0;
pthread_t self = pthread_self ();
ptw32_thread_t * sp = (ptw32_thread_t *) self.p;
if (sp == NULL
|| (type != PTHREAD_CANCEL_DEFERRED
&& type != PTHREAD_CANCEL_ASYNCHRONOUS))
{
return EINVAL;
}
ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
if (oldtype != NULL)
{
*oldtype = sp->cancelType;
}
sp->cancelType = type;
if (sp->cancelState == PTHREAD_CANCEL_ENABLE
&& type == PTHREAD_CANCEL_ASYNCHRONOUS
&& WaitForSingleObject (sp->cancelEvent, 0) == WAIT_OBJECT_0)
{
sp->state = PThreadStateCanceling;
sp->cancelState = PTHREAD_CANCEL_DISABLE;
ResetEvent (sp->cancelEvent);
ptw32_mcs_lock_release (&stateLock);
ptw32_throw (PTW32_EPS_CANCEL);
}
ptw32_mcs_lock_release (&stateLock);
return (result);
} 
