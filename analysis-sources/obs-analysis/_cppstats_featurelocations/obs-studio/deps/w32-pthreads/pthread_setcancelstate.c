



































#include "pthread.h"
#include "implement.h"


int
pthread_setcancelstate (int state, int *oldstate)






































{
ptw32_mcs_local_node_t stateLock;
int result = 0;
pthread_t self = pthread_self ();
ptw32_thread_t * sp = (ptw32_thread_t *) self.p;

if (sp == NULL
|| (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE))
{
return EINVAL;
}




ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);

if (oldstate != NULL)
{
*oldstate = sp->cancelState;
}

sp->cancelState = state;




if (state == PTHREAD_CANCEL_ENABLE
&& sp->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
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
