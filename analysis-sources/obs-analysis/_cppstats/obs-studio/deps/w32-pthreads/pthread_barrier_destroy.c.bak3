



































#include "pthread.h"
#include "implement.h"

int
pthread_barrier_destroy (pthread_barrier_t * barrier)
{
int result = 0;
pthread_barrier_t b;
ptw32_mcs_local_node_t node;

if (barrier == NULL || *barrier == (pthread_barrier_t) PTW32_OBJECT_INVALID)
{
return EINVAL;
}

if (0 != ptw32_mcs_lock_try_acquire(&(*barrier)->lock, &node))
{
return EBUSY;
}

b = *barrier;

if (b->nCurrentBarrierHeight < b->nInitialBarrierHeight)
{
result = EBUSY;
}
else
{
if (0 == (result = sem_destroy (&(b->semBarrierBreeched))))
{
*barrier = (pthread_barrier_t) PTW32_OBJECT_INVALID;










ptw32_mcs_lock_release(&node);
(void) free (b);
return 0;
}
else
{




(void) sem_init (&(b->semBarrierBreeched), b->pshared, 0);
}

if (result != 0)
{




result = EBUSY;
}
}

ptw32_mcs_lock_release(&node);
return (result);
}
