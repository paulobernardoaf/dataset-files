



































#include <limits.h>

#include "pthread.h"
#include "implement.h"

int
pthread_rwlock_trywrlock (pthread_rwlock_t * rwlock)
{
int result, result1;
pthread_rwlock_t rwl;

if (rwlock == NULL || *rwlock == NULL)
{
return EINVAL;
}







if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
{
result = ptw32_rwlock_check_need_init (rwlock);

if (result != 0 && result != EBUSY)
{
return result;
}
}

rwl = *rwlock;

if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
{
return EINVAL;
}

if ((result = pthread_mutex_trylock (&(rwl->mtxExclusiveAccess))) != 0)
{
return result;
}

if ((result =
pthread_mutex_trylock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
result1 = pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return ((result1 != 0) ? result1 : result);
}

if (rwl->nExclusiveAccessCount == 0)
{
if (rwl->nCompletedSharedAccessCount > 0)
{
rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
rwl->nCompletedSharedAccessCount = 0;
}

if (rwl->nSharedAccessCount > 0)
{
if ((result =
pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return result;
}

if ((result =
pthread_mutex_unlock (&(rwl->mtxExclusiveAccess))) == 0)
{
result = EBUSY;
}
}
else
{
rwl->nExclusiveAccessCount = 1;
}
}
else
{
result = EBUSY;
}

return result;
}
