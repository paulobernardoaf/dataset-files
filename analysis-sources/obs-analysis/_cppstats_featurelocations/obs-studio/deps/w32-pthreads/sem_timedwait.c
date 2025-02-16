










































#include "pthread.h"
#include "semaphore.h"
#include "implement.h"


typedef struct {
sem_t sem;
int * resultPtr;
} sem_timedwait_cleanup_args_t;


static void PTW32_CDECL
ptw32_sem_timedwait_cleanup (void * args)
{
sem_timedwait_cleanup_args_t * a = (sem_timedwait_cleanup_args_t *)args;
sem_t s = a->sem;

if (pthread_mutex_lock (&s->lock) == 0)
{








if (WaitForSingleObject(s->sem, 0) == WAIT_OBJECT_0)
{

*(a->resultPtr) = 0;
}
else
{

s->value++;
#if defined(NEED_SEM)
if (s->value > 0)
{
s->leftToUnblock = 0;
}
#else




#endif
}
(void) pthread_mutex_unlock (&s->lock);
}
}


int
sem_timedwait (sem_t * sem, const struct timespec *abstime)





































{
int result = 0;
sem_t s = *sem;

pthread_testcancel();

if (sem == NULL)
{
result = EINVAL;
}
else
{
DWORD milliseconds;

if (abstime == NULL)
{
milliseconds = INFINITE;
}
else
{



milliseconds = ptw32_relmillisecs (abstime);
}

if ((result = pthread_mutex_lock (&s->lock)) == 0)
{
int v;



if (*sem == NULL)
{
(void) pthread_mutex_unlock (&s->lock);
errno = EINVAL;
return -1;
}

v = --s->value;
(void) pthread_mutex_unlock (&s->lock);

if (v < 0)
{
#if defined(NEED_SEM)
int timedout;
#endif
sem_timedwait_cleanup_args_t cleanup_args;

cleanup_args.sem = s;
cleanup_args.resultPtr = &result;

#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth(0)
#endif

pthread_cleanup_push(ptw32_sem_timedwait_cleanup, (void *) &cleanup_args);
#if defined(NEED_SEM)
timedout =
#endif
result = pthreadCancelableTimedWait (s->sem, milliseconds);
pthread_cleanup_pop(result);
#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth()
#endif

#if defined(NEED_SEM)

if (!timedout && pthread_mutex_lock (&s->lock) == 0)
{
if (*sem == NULL)
{
(void) pthread_mutex_unlock (&s->lock);
errno = EINVAL;
return -1;
}

if (s->leftToUnblock > 0)
{
--s->leftToUnblock;
SetEvent(s->sem);
}
(void) pthread_mutex_unlock (&s->lock);
}

#endif 

}
}

}

if (result != 0)
{

errno = result;
return -1;

}

return 0;

} 
