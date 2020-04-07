




































#include "pthread.h"
#include "implement.h"


void
ptw32_threadDestroy (pthread_t thread)
{
ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;
ptw32_thread_t threadCopy;

if (tp != NULL)
{



memcpy (&threadCopy, tp, sizeof (threadCopy));





ptw32_threadReusePush (thread);


if (threadCopy.cancelEvent != NULL)
{
CloseHandle (threadCopy.cancelEvent);
}

#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)



if (threadCopy.threadH != 0)
{
CloseHandle (threadCopy.threadH);
}
#endif

}
} 

