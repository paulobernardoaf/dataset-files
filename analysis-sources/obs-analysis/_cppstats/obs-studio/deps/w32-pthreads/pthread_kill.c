#include "pthread.h"
#include "implement.h"
#if !defined(WINCE)
#include <signal.h>
#endif
int
pthread_kill (pthread_t thread, int sig)
{
int result = 0;
ptw32_thread_t * tp;
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);
tp = (ptw32_thread_t *) thread.p;
if (NULL == tp
|| thread.x != tp->ptHandle.x
|| NULL == tp->threadH)
{
result = ESRCH;
}
ptw32_mcs_lock_release(&node);
if (0 == result && 0 != sig)
{
result = EINVAL;
}
return result;
} 
