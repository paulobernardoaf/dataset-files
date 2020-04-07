




































#include "pthread.h"
#include "implement.h"


int
pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)


























{
int result;
pthread_cond_t cv = NULL;

if (cond == NULL)
{
return EINVAL;
}

if ((attr != NULL && *attr != NULL) &&
((*attr)->pshared == PTHREAD_PROCESS_SHARED))
{




result = ENOSYS;
goto DONE;
}

cv = (pthread_cond_t) calloc (1, sizeof (*cv));

if (cv == NULL)
{
result = ENOMEM;
goto DONE;
}

cv->nWaitersBlocked = 0;
cv->nWaitersToUnblock = 0;
cv->nWaitersGone = 0;

if (sem_init (&(cv->semBlockLock), 0, 1) != 0)
{
result = errno;
goto FAIL0;
}

if (sem_init (&(cv->semBlockQueue), 0, 0) != 0)
{
result = errno;
goto FAIL1;
}

if ((result = pthread_mutex_init (&(cv->mtxUnblockLock), 0)) != 0)
{
goto FAIL2;
}

result = 0;

goto DONE;






FAIL2:
(void) sem_destroy (&(cv->semBlockQueue));

FAIL1:
(void) sem_destroy (&(cv->semBlockLock));

FAIL0:
(void) free (cv);
cv = NULL;

DONE:
if (0 == result)
{
ptw32_mcs_local_node_t node;

ptw32_mcs_lock_acquire(&ptw32_cond_list_lock, &node);

cv->next = NULL;
cv->prev = ptw32_cond_list_tail;

if (ptw32_cond_list_tail != NULL)
{
ptw32_cond_list_tail->next = cv;
}

ptw32_cond_list_tail = cv;

if (ptw32_cond_list_head == NULL)
{
ptw32_cond_list_head = cv;
}

ptw32_mcs_lock_release(&node);
}

*cond = cv;

return result;

} 
