



































#include "pthread.h"
#include "implement.h"

pthread_t
pthread_self (void)



















{
pthread_t self;
pthread_t nil = {NULL, 0};
ptw32_thread_t * sp;

#if defined(_UWIN)
if (!ptw32_selfThreadKey)
return nil;
#endif

sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);

if (sp != NULL)
{
self = sp->ptHandle;
}
else
{




self = ptw32_new ();
sp = (ptw32_thread_t *) self.p;

if (sp != NULL)
{






sp->implicit = 1;
sp->detachState = PTHREAD_CREATE_DETACHED;
sp->thread = GetCurrentThreadId ();

#if defined(NEED_DUPLICATEHANDLE)









sp->threadH = GetCurrentThread ();
#else
if (!DuplicateHandle (GetCurrentProcess (),
GetCurrentThread (),
GetCurrentProcess (),
&sp->threadH,
0, FALSE, DUPLICATE_SAME_ACCESS))
{





ptw32_threadReusePush (self);




return nil;
}
#endif





sp->sched_priority = GetThreadPriority (sp->threadH);
pthread_setspecific (ptw32_selfThreadKey, (void *) sp);
}
}

return (self);

} 
