#include <limits.h>
#include "pthread.h"
#include "implement.h"
int
pthread_rwlock_timedrdlock (pthread_rwlock_t * rwlock,
const struct timespec *abstime)
{
int result;
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
if ((result =
pthread_mutex_timedlock (&(rwl->mtxExclusiveAccess), abstime)) != 0)
{
return result;
}
if (++rwl->nSharedAccessCount == INT_MAX)
{
if ((result =
pthread_mutex_timedlock (&(rwl->mtxSharedAccessCompleted),
abstime)) != 0)
{
if (result == ETIMEDOUT)
{
++rwl->nCompletedSharedAccessCount;
}
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return result;
}
rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
rwl->nCompletedSharedAccessCount = 0;
if ((result =
pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return result;
}
}
return (pthread_mutex_unlock (&(rwl->mtxExclusiveAccess)));
}
