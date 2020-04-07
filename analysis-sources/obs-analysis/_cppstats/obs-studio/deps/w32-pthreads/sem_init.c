#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
int
sem_init (sem_t * sem, int pshared, unsigned int value)
{
int result = 0;
sem_t s = NULL;
if (pshared != 0)
{
result = EPERM;
}
else if (value > (unsigned int)SEM_VALUE_MAX)
{
result = EINVAL;
}
else
{
s = (sem_t) calloc (1, sizeof (*s));
if (NULL == s)
{
result = ENOMEM;
}
else
{
s->value = value;
if (pthread_mutex_init(&s->lock, NULL) == 0)
{
#if defined(NEED_SEM)
s->sem = CreateEvent (NULL,
PTW32_FALSE, 
PTW32_FALSE, 
NULL);
if (0 == s->sem)
{
free (s);
(void) pthread_mutex_destroy(&s->lock);
result = ENOSPC;
}
else
{
s->leftToUnblock = 0;
}
#else 
if ((s->sem = CreateSemaphore (NULL, 
(long) 0, 
(long) SEM_VALUE_MAX, 
NULL)) == 0) 
{
(void) pthread_mutex_destroy(&s->lock);
result = ENOSPC;
}
#endif 
}
else
{
result = ENOSPC;
}
if (result != 0)
{
free(s);
}
}
}
if (result != 0)
{
errno = result;
return -1;
}
*sem = s;
return 0;
} 
