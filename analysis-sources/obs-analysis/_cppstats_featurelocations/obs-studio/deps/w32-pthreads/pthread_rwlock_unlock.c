



































#include <limits.h>

#include "pthread.h"
#include "implement.h"

int
pthread_rwlock_unlock (pthread_rwlock_t * rwlock)
{
int result, result1;
pthread_rwlock_t rwl;

if (rwlock == NULL || *rwlock == NULL)
{
return (EINVAL);
}

if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
{



return 0;
}

rwl = *rwlock;

if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
{
return EINVAL;
}

if (rwl->nExclusiveAccessCount == 0)
{
if ((result =
pthread_mutex_lock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
return result;
}

if (++rwl->nCompletedSharedAccessCount == 0)
{
result = pthread_cond_signal (&(rwl->cndSharedAccessCompleted));
}

result1 = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
}
else
{
rwl->nExclusiveAccessCount--;

result = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
result1 = pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));

}

return ((result != 0) ? result : result1);
}
