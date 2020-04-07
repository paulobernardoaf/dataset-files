#include <limits.h>
#include "pthread.h"
#include "implement.h"
int
pthread_rwlock_timedwrlock (pthread_rwlock_t * rwlock,
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
if ((result =
pthread_mutex_timedlock (&(rwl->mtxSharedAccessCompleted),
abstime)) != 0)
{
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return result;
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
rwl->nCompletedSharedAccessCount = -rwl->nSharedAccessCount;
#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth(0)
#endif
pthread_cleanup_push (ptw32_rwlock_cancelwrwait, (void *) rwl);
do
{
result =
pthread_cond_timedwait (&(rwl->cndSharedAccessCompleted),
&(rwl->mtxSharedAccessCompleted),
abstime);
}
while (result == 0 && rwl->nCompletedSharedAccessCount < 0);
pthread_cleanup_pop ((result != 0) ? 1 : 0);
#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth()
#endif
if (result == 0)
{
rwl->nSharedAccessCount = 0;
}
}
}
if (result == 0)
{
rwl->nExclusiveAccessCount++;
}
return result;
}
