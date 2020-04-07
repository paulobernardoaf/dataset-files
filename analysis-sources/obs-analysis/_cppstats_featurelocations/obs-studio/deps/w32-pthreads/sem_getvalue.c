










































#include "pthread.h"
#include "semaphore.h"
#include "implement.h"


int
sem_getvalue (sem_t * sem, int *sval)




























{
if (sem == NULL || *sem == NULL || sval == NULL)
{
errno = EINVAL;
return -1;
}
else
{
long value;
register sem_t s = *sem;
int result = 0;

if ((result = pthread_mutex_lock(&s->lock)) == 0)
{


if (*sem == NULL)
{
(void) pthread_mutex_unlock (&s->lock);
errno = EINVAL;
return -1;
}

value = s->value;
(void) pthread_mutex_unlock(&s->lock);
*sval = value;
}

return result;
}

} 
