



































#include <limits.h>

#include "pthread.h"
#include "implement.h"

int
pthread_rwlock_destroy (pthread_rwlock_t * rwlock)
{
pthread_rwlock_t rwl;
int result = 0, result1 = 0, result2 = 0;

if (rwlock == NULL || *rwlock == NULL)
{
return EINVAL;
}

if (*rwlock != PTHREAD_RWLOCK_INITIALIZER)
{
rwl = *rwlock;

if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
{
return EINVAL;
}

if ((result = pthread_mutex_lock (&(rwl->mtxExclusiveAccess))) != 0)
{
return result;
}

if ((result =
pthread_mutex_lock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
(void) pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
return result;
}





if (rwl->nExclusiveAccessCount > 0
|| rwl->nSharedAccessCount > rwl->nCompletedSharedAccessCount)
{
result = pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted));
result1 = pthread_mutex_unlock (&(rwl->mtxExclusiveAccess));
result2 = EBUSY;
}
else
{
rwl->nMagic = 0;

if ((result =
pthread_mutex_unlock (&(rwl->mtxSharedAccessCompleted))) != 0)
{
pthread_mutex_unlock (&rwl->mtxExclusiveAccess);
return result;
}

if ((result =
pthread_mutex_unlock (&(rwl->mtxExclusiveAccess))) != 0)
{
return result;
}

*rwlock = NULL; 
result = pthread_cond_destroy (&(rwl->cndSharedAccessCompleted));
result1 = pthread_mutex_destroy (&(rwl->mtxSharedAccessCompleted));
result2 = pthread_mutex_destroy (&(rwl->mtxExclusiveAccess));
(void) free (rwl);
}
}
else
{
ptw32_mcs_local_node_t node;



ptw32_mcs_lock_acquire(&ptw32_rwlock_test_init_lock, &node);




if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
{






*rwlock = NULL;
}
else
{




result = EBUSY;
}

ptw32_mcs_lock_release(&node);
}

return ((result != 0) ? result : ((result1 != 0) ? result1 : result2));
}
