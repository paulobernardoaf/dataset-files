



































#include "pthread.h"
#include "implement.h"










































int
pthread_delay_np (struct timespec *interval)
{
DWORD wait_time;
DWORD secs_in_millisecs;
DWORD millisecs;
DWORD status;
pthread_t self;
ptw32_thread_t * sp;

if (interval == NULL)
{
return EINVAL;
}

if (interval->tv_sec == 0L && interval->tv_nsec == 0L)
{
pthread_testcancel ();
Sleep (0);
pthread_testcancel ();
return (0);
}


secs_in_millisecs = (DWORD)interval->tv_sec * 1000L;


millisecs = (interval->tv_nsec + 999999L) / 1000000L;

#if defined(__WATCOMC__)
#pragma disable_message (124)
#endif






if (0 > (wait_time = secs_in_millisecs + millisecs))
{
return EINVAL;
}

#if defined(__WATCOMC__)
#pragma enable_message (124)
#endif

if (NULL == (self = pthread_self ()).p)
{
return ENOMEM;
}

sp = (ptw32_thread_t *) self.p;

if (sp->cancelState == PTHREAD_CANCEL_ENABLE)
{




if (WAIT_OBJECT_0 ==
(status = WaitForSingleObject (sp->cancelEvent, wait_time)))
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
return ESRCH;
}
else if (status != WAIT_TIMEOUT)
{
return EINVAL;
}
}
else
{
Sleep (wait_time);
}

return (0);
}
