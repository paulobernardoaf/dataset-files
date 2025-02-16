



































#include "pthread.h"
#include "implement.h"


int
pthread_barrier_init (pthread_barrier_t * barrier,
const pthread_barrierattr_t * attr, unsigned int count)
{
pthread_barrier_t b;

if (barrier == NULL || count == 0)
{
return EINVAL;
}

if (NULL != (b = (pthread_barrier_t) calloc (1, sizeof (*b))))
{
b->pshared = (attr != NULL && *attr != NULL
? (*attr)->pshared : PTHREAD_PROCESS_PRIVATE);

b->nCurrentBarrierHeight = b->nInitialBarrierHeight = count;
b->lock = 0;

if (0 == sem_init (&(b->semBarrierBreeched), b->pshared, 0))
{
*barrier = b;
return 0;
}
(void) free (b);
}

return ENOMEM;
}
