










































#include "pthread.h"
#include "semaphore.h"
#include "implement.h"


int
sem_destroy (sem_t * sem)























{
int result = 0;
sem_t s = NULL;

if (sem == NULL || *sem == NULL)
{
result = EINVAL;
}
else
{
s = *sem;

if ((result = pthread_mutex_lock (&s->lock)) == 0)
{
if (s->value < 0)
{
(void) pthread_mutex_unlock (&s->lock);
result = EBUSY;
}
else
{


if (!CloseHandle (s->sem))
{
(void) pthread_mutex_unlock (&s->lock);
result = EINVAL;
}
else
{








*sem = NULL;



s->value = SEM_VALUE_MAX;

(void) pthread_mutex_unlock (&s->lock);

do
{




Sleep(0);
}
while (pthread_mutex_destroy (&s->lock) == EBUSY);
}
}
}
}

if (result != 0)
{
errno = result;
return -1;
}

free (s);

return 0;

} 
