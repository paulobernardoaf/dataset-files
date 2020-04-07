#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
int
sem_post_multiple (sem_t * sem, int count)
{
int result = 0;
long waiters;
sem_t s = *sem;
if (s == NULL || count <= 0)
{
result = EINVAL;
}
else if ((result = pthread_mutex_lock (&s->lock)) == 0)
{
if (*sem == NULL)
{
(void) pthread_mutex_unlock (&s->lock);
result = EINVAL;
return -1;
}
if (s->value <= (SEM_VALUE_MAX - count))
{
waiters = -s->value;
s->value += count;
if (waiters > 0)
{
#if defined(NEED_SEM)
if (SetEvent(s->sem))
{
waiters--;
s->leftToUnblock += count - 1;
if (s->leftToUnblock > waiters)
{
s->leftToUnblock = waiters;
}
}
#else
if (ReleaseSemaphore (s->sem, (waiters<=count)?waiters:count, 0))
{
}
#endif
else
{
s->value -= count;
result = EINVAL;
}
}
}
else
{
result = ERANGE;
}
(void) pthread_mutex_unlock (&s->lock);
}
if (result != 0)
{
errno = result;
return -1;
}
return 0;
} 
