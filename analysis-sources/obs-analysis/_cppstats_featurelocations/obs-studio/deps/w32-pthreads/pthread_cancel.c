



































#include "pthread.h"
#include "implement.h"
#include "context.h"

static void
ptw32_cancel_self (void)
{
ptw32_throw (PTW32_EPS_CANCEL);


}

static void CALLBACK
ptw32_cancel_callback (ULONG_PTR unused)
{
ptw32_throw (PTW32_EPS_CANCEL);


}






DWORD
ptw32_RegisterCancelation (PAPCFUNC unused1, HANDLE threadH, DWORD unused2)
{
CONTEXT context;

context.ContextFlags = CONTEXT_CONTROL;
GetThreadContext (threadH, &context);
PTW32_PROGCTR (context) = (DWORD_PTR) ptw32_cancel_self;
SetThreadContext (threadH, &context);
return 0;
}

int
pthread_cancel (pthread_t thread)





















{
int result;
int cancel_self;
pthread_t self;
ptw32_thread_t * tp;
ptw32_mcs_local_node_t stateLock;

result = pthread_kill (thread, 0);

if (0 != result)
{
return result;
}

if ((self = pthread_self ()).p == NULL)
{
return ENOMEM;
};







cancel_self = pthread_equal (thread, self);

tp = (ptw32_thread_t *) thread.p;




ptw32_mcs_lock_acquire (&tp->stateLock, &stateLock);

if (tp->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
&& tp->cancelState == PTHREAD_CANCEL_ENABLE
&& tp->state < PThreadStateCanceling)
{
if (cancel_self)
{
tp->state = PThreadStateCanceling;
tp->cancelState = PTHREAD_CANCEL_DISABLE;

ptw32_mcs_lock_release (&stateLock);
ptw32_throw (PTW32_EPS_CANCEL);


}
else
{
HANDLE threadH = tp->threadH;

SuspendThread (threadH);

if (WaitForSingleObject (threadH, 0) == WAIT_TIMEOUT)
{
tp->state = PThreadStateCanceling;
tp->cancelState = PTHREAD_CANCEL_DISABLE;






ptw32_register_cancelation ((PAPCFUNC)ptw32_cancel_callback, threadH, 0);
ptw32_mcs_lock_release (&stateLock);
ResumeThread (threadH);
}
}
}
else
{



if (tp->state < PThreadStateCancelPending)
{
tp->state = PThreadStateCancelPending;
if (!SetEvent (tp->cancelEvent))
{
result = ESRCH;
}
}
else if (tp->state >= PThreadStateCanceling)
{
result = ESRCH;
}

ptw32_mcs_lock_release (&stateLock);
}

return (result);
}
