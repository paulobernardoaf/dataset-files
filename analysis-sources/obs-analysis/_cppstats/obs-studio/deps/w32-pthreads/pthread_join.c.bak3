




































#include "pthread.h"
#include "implement.h"





#if !defined(WINCE)
#include <signal.h>
#endif


int
pthread_join (pthread_t thread, void **value_ptr)
































{
int result;
pthread_t self;
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
result = 0;
}

ptw32_mcs_lock_release(&node);

if (result == 0)
{



self = pthread_self();

if (NULL == self.p)
{
result = ENOENT;
}
else if (pthread_equal (self, thread))
{
result = EDEADLK;
}
else
{







result = pthreadCancelableWait (tp->threadH);

if (0 == result)
{
if (value_ptr != NULL)
{
*value_ptr = tp->exitStatus;
}






result = pthread_detach (thread);
}
else
{
result = ESRCH;
}
}
}

return (result);

} 
