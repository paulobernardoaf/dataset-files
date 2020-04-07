#include "pthread.h"
#include "implement.h"
pthread_t
ptw32_threadReusePop (void)
{
pthread_t t = {NULL, 0};
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);
if (PTW32_THREAD_REUSE_EMPTY != ptw32_threadReuseTop)
{
ptw32_thread_t * tp;
tp = ptw32_threadReuseTop;
ptw32_threadReuseTop = tp->prevReuse;
if (PTW32_THREAD_REUSE_EMPTY == ptw32_threadReuseTop)
{
ptw32_threadReuseBottom = PTW32_THREAD_REUSE_EMPTY;
}
tp->prevReuse = NULL;
t = tp->ptHandle;
}
ptw32_mcs_lock_release(&node);
return t;
}
void
ptw32_threadReusePush (pthread_t thread)
{
ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;
pthread_t t;
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);
t = tp->ptHandle;
memset(tp, 0, sizeof(ptw32_thread_t));
tp->ptHandle = t;
#if defined(PTW32_THREAD_ID_REUSE_INCREMENT)
tp->ptHandle.x += PTW32_THREAD_ID_REUSE_INCREMENT;
#else
tp->ptHandle.x++;
#endif
tp->state = PThreadStateReuse;
tp->prevReuse = PTW32_THREAD_REUSE_EMPTY;
if (PTW32_THREAD_REUSE_EMPTY != ptw32_threadReuseBottom)
{
ptw32_threadReuseBottom->prevReuse = tp;
}
else
{
ptw32_threadReuseTop = tp;
}
ptw32_threadReuseBottom = tp;
ptw32_mcs_lock_release(&node);
}
