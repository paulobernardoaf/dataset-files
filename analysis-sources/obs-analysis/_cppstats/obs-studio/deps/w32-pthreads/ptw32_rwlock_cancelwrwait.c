#include "pthread.h"
#include "implement.h"
void
ptw32_rwlock_cancelwrwait (void *arg)
{
pthread_rwlock_t rwl = (pthread_rwlock_t) arg;
rwl->nSharedAccessCount = -rwl->nCompletedSharedAccessCount;
rwl->nCompletedSharedAccessCount = 0;
(void) pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
}
