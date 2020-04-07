



































#include "pthread.h"
#include "implement.h"


void
pthread_testcancel (void)



























{
ptw32_mcs_local_node_t stateLock;
pthread_t self = pthread_self ();
ptw32_thread_t * sp = (ptw32_thread_t *) self.p;

if (sp == NULL)
{
return;
}






if (sp->state != PThreadStateCancelPending)
{
return;
}

ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);

if (sp->cancelState != PTHREAD_CANCEL_DISABLE)
{
ResetEvent(sp->cancelEvent);
sp->state = PThreadStateCanceling;
sp->cancelState = PTHREAD_CANCEL_DISABLE;
ptw32_mcs_lock_release (&stateLock);
ptw32_throw (PTW32_EPS_CANCEL);

}

ptw32_mcs_lock_release (&stateLock);
} 
