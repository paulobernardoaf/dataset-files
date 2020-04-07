#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
int
sem_post (sem_t * sem)
{
int result = 0;
sem_t s = *sem;
if (s == NULL)
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
if (s->value < SEM_VALUE_MAX)
{
#if defined(NEED_SEM)
if (++s->value <= 0
&& !SetEvent(s->sem))
{
s->value--;
result = EINVAL;
}
#else
if (++s->value <= 0
&& !ReleaseSemaphore (s->sem, 1, NULL))
{
s->value--;
result = EINVAL;
}
#endif 
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
