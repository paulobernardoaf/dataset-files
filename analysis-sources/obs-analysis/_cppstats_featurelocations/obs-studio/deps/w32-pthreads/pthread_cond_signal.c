








































#include "pthread.h"
#include "implement.h"

static INLINE int
ptw32_cond_unblock (pthread_cond_t * cond, int unblockAll)



















{
int result;
pthread_cond_t cv;
int nSignalsToIssue;

if (cond == NULL || *cond == NULL)
{
return EINVAL;
}

cv = *cond;





if (cv == PTHREAD_COND_INITIALIZER)
{
return 0;
}

if ((result = pthread_mutex_lock (&(cv->mtxUnblockLock))) != 0)
{
return result;
}

if (0 != cv->nWaitersToUnblock)
{
if (0 == cv->nWaitersBlocked)
{
return pthread_mutex_unlock (&(cv->mtxUnblockLock));
}
if (unblockAll)
{
cv->nWaitersToUnblock += (nSignalsToIssue = cv->nWaitersBlocked);
cv->nWaitersBlocked = 0;
}
else
{
nSignalsToIssue = 1;
cv->nWaitersToUnblock++;
cv->nWaitersBlocked--;
}
}
else if (cv->nWaitersBlocked > cv->nWaitersGone)
{

if (ptw32_semwait (&(cv->semBlockLock)) != 0)
{
result = errno;
(void) pthread_mutex_unlock (&(cv->mtxUnblockLock));
return result;
}
if (0 != cv->nWaitersGone)
{
cv->nWaitersBlocked -= cv->nWaitersGone;
cv->nWaitersGone = 0;
}
if (unblockAll)
{
nSignalsToIssue = cv->nWaitersToUnblock = cv->nWaitersBlocked;
cv->nWaitersBlocked = 0;
}
else
{
nSignalsToIssue = cv->nWaitersToUnblock = 1;
cv->nWaitersBlocked--;
}
}
else
{
return pthread_mutex_unlock (&(cv->mtxUnblockLock));
}

if ((result = pthread_mutex_unlock (&(cv->mtxUnblockLock))) == 0)
{
if (sem_post_multiple (&(cv->semBlockQueue), nSignalsToIssue) != 0)
{
result = errno;
}
}

return result;

} 

int
pthread_cond_signal (pthread_cond_t * cond)
































{



return (ptw32_cond_unblock (cond, 0));

} 

int
pthread_cond_broadcast (pthread_cond_t * cond)





























{



return (ptw32_cond_unblock (cond, PTW32_TRUE));

} 
