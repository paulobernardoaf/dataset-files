#include "pthread.h"
#include "implement.h"
int
pthread_cond_destroy (pthread_cond_t * cond)
{
pthread_cond_t cv;
int result = 0, result1 = 0, result2 = 0;
if (cond == NULL || *cond == NULL)
{
return EINVAL;
}
if (*cond != PTHREAD_COND_INITIALIZER)
{
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_cond_list_lock, &node);
cv = *cond;
if (ptw32_semwait (&(cv->semBlockLock)) != 0) 
{
result = errno;
}
else
{
if ((result = pthread_mutex_trylock (&(cv->mtxUnblockLock))) != 0)
{
(void) sem_post (&(cv->semBlockLock));
}
}
if (result != 0)
{
ptw32_mcs_lock_release(&node);
return result;
}
if (cv->nWaitersBlocked > cv->nWaitersGone)
{
if (sem_post (&(cv->semBlockLock)) != 0)
{
result = errno;
}
result1 = pthread_mutex_unlock (&(cv->mtxUnblockLock));
result2 = EBUSY;
}
else
{
*cond = NULL;
if (sem_destroy (&(cv->semBlockLock)) != 0)
{
result = errno;
}
if (sem_destroy (&(cv->semBlockQueue)) != 0)
{
result1 = errno;
}
if ((result2 = pthread_mutex_unlock (&(cv->mtxUnblockLock))) == 0)
{
result2 = pthread_mutex_destroy (&(cv->mtxUnblockLock));
}
if (ptw32_cond_list_head == cv)
{
ptw32_cond_list_head = cv->next;
}
else
{
cv->prev->next = cv->next;
}
if (ptw32_cond_list_tail == cv)
{
ptw32_cond_list_tail = cv->prev;
}
else
{
cv->next->prev = cv->prev;
}
(void) free (cv);
}
ptw32_mcs_lock_release(&node);
}
else
{
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_cond_test_init_lock, &node);
if (*cond == PTHREAD_COND_INITIALIZER)
{
*cond = NULL;
}
else
{
result = EBUSY;
}
ptw32_mcs_lock_release(&node);
}
return ((result != 0) ? result : ((result1 != 0) ? result1 : result2));
}
