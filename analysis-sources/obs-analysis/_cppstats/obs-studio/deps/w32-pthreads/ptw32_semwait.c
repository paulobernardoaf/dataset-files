#if !defined(_UWIN)
#endif
#include "pthread.h"
#include "implement.h"
int
ptw32_semwait (sem_t * sem)
{
int result = 0;
sem_t s = *sem;
if (s == NULL)
{
result = EINVAL;
}
else
{
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
if (WaitForSingleObject (s->sem, INFINITE) == WAIT_OBJECT_0)
{
#if defined(NEED_SEM)
if (pthread_mutex_lock (&s->lock) == 0)
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
return 0;
}
}
else
{
return 0;
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
