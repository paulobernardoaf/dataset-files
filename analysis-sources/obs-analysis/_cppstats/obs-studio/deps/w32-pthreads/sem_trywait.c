#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
int
sem_trywait (sem_t * sem)
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
errno = EINVAL;
return -1;
}
if (s->value > 0)
{
s->value--;
}
else
{
result = EAGAIN;
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
