#include "pthread.h"
#include "implement.h"
void
ptw32_processTerminate (void)
{
if (ptw32_processInitialized)
{
ptw32_thread_t * tp, * tpNext;
ptw32_mcs_local_node_t node;
if (ptw32_selfThreadKey != NULL)
{
pthread_key_delete (ptw32_selfThreadKey);
ptw32_selfThreadKey = NULL;
}
if (ptw32_cleanupKey != NULL)
{
pthread_key_delete (ptw32_cleanupKey);
ptw32_cleanupKey = NULL;
}
ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);
tp = ptw32_threadReuseTop;
while (tp != PTW32_THREAD_REUSE_EMPTY)
{
tpNext = tp->prevReuse;
free (tp);
tp = tpNext;
}
ptw32_mcs_lock_release(&node);
ptw32_processInitialized = PTW32_FALSE;
}
} 
