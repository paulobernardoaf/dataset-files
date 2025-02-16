



































































































































































































































































#include "pthread.h"
#include "implement.h"





typedef struct
{
pthread_mutex_t *mutexPtr;
pthread_cond_t cv;
int *resultPtr;
} ptw32_cond_wait_cleanup_args_t;

static void PTW32_CDECL
ptw32_cond_wait_cleanup (void *args)
{
ptw32_cond_wait_cleanup_args_t *cleanup_args =
(ptw32_cond_wait_cleanup_args_t *) args;
pthread_cond_t cv = cleanup_args->cv;
int *resultPtr = cleanup_args->resultPtr;
int nSignalsWasLeft;
int result;







if ((result = pthread_mutex_lock (&(cv->mtxUnblockLock))) != 0)
{
*resultPtr = result;
return;
}

if (0 != (nSignalsWasLeft = cv->nWaitersToUnblock))
{
--(cv->nWaitersToUnblock);
}
else if (INT_MAX / 2 == ++(cv->nWaitersGone))
{

if (ptw32_semwait (&(cv->semBlockLock)) != 0)
{
*resultPtr = errno;





return;
}
cv->nWaitersBlocked -= cv->nWaitersGone;
if (sem_post (&(cv->semBlockLock)) != 0)
{
*resultPtr = errno;





return;
}
cv->nWaitersGone = 0;
}

if ((result = pthread_mutex_unlock (&(cv->mtxUnblockLock))) != 0)
{
*resultPtr = result;
return;
}

if (1 == nSignalsWasLeft)
{
if (sem_post (&(cv->semBlockLock)) != 0)
{
*resultPtr = errno;
return;
}
}





if ((result = pthread_mutex_lock (cleanup_args->mutexPtr)) != 0)
{
*resultPtr = result;
}
} 

static INLINE int
ptw32_cond_timedwait (pthread_cond_t * cond,
pthread_mutex_t * mutex, const struct timespec *abstime)
{
int result = 0;
pthread_cond_t cv;
ptw32_cond_wait_cleanup_args_t cleanup_args;

if (cond == NULL || *cond == NULL)
{
return EINVAL;
}







if (*cond == PTHREAD_COND_INITIALIZER)
{
result = ptw32_cond_check_need_init (cond);
}

if (result != 0 && result != EBUSY)
{
return result;
}

cv = *cond;


if (sem_wait (&(cv->semBlockLock)) != 0)
{
return errno;
}

++(cv->nWaitersBlocked);

if (sem_post (&(cv->semBlockLock)) != 0)
{
return errno;
}




cleanup_args.mutexPtr = mutex;
cleanup_args.cv = cv;
cleanup_args.resultPtr = &result;

#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth(0)
#endif
pthread_cleanup_push (ptw32_cond_wait_cleanup, (void *) &cleanup_args);




if ((result = pthread_mutex_unlock (mutex)) == 0)
{

















if (sem_timedwait (&(cv->semBlockQueue), abstime) != 0)
{
result = errno;
}
}




pthread_cleanup_pop (1);
#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth()
#endif




return result;

} 


int
pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)















































{



return (ptw32_cond_timedwait (cond, mutex, NULL));

} 


int
pthread_cond_timedwait (pthread_cond_t * cond,
pthread_mutex_t * mutex,
const struct timespec *abstime)












































{
if (abstime == NULL)
{
return EINVAL;
}

return (ptw32_cond_timedwait (cond, mutex, abstime));

} 
