#include <limits.h>
#include "pthread.h"
#include "implement.h"
int
pthread_rwlock_init (pthread_rwlock_t * rwlock,
const pthread_rwlockattr_t * attr)
{
int result;
pthread_rwlock_t rwl = 0;
if (rwlock == NULL)
{
return EINVAL;
}
if (attr != NULL && *attr != NULL)
{
result = EINVAL; 
goto DONE;
}
rwl = (pthread_rwlock_t) calloc (1, sizeof (*rwl));
if (rwl == NULL)
{
result = ENOMEM;
goto DONE;
}
rwl->nSharedAccessCount = 0;
rwl->nExclusiveAccessCount = 0;
rwl->nCompletedSharedAccessCount = 0;
result = pthread_mutex_init (&rwl->mtxExclusiveAccess, NULL);
if (result != 0)
{
goto FAIL0;
}
result = pthread_mutex_init (&rwl->mtxSharedAccessCompleted, NULL);
if (result != 0)
{
goto FAIL1;
}
result = pthread_cond_init (&rwl->cndSharedAccessCompleted, NULL);
if (result != 0)
{
goto FAIL2;
}
rwl->nMagic = PTW32_RWLOCK_MAGIC;
result = 0;
goto DONE;
FAIL2:
(void) pthread_mutex_destroy (&(rwl->mtxSharedAccessCompleted));
FAIL1:
(void) pthread_mutex_destroy (&(rwl->mtxExclusiveAccess));
FAIL0:
(void) free (rwl);
rwl = NULL;
DONE:
*rwlock = rwl;
return result;
}
