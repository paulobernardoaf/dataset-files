#include "pthread.h"
#include "implement.h"
int
pthread_barrier_wait (pthread_barrier_t * barrier)
{
int result;
pthread_barrier_t b;
ptw32_mcs_local_node_t node;
if (barrier == NULL || *barrier == (pthread_barrier_t) PTW32_OBJECT_INVALID)
{
return EINVAL;
}
ptw32_mcs_lock_acquire(&(*barrier)->lock, &node);
b = *barrier;
if (--b->nCurrentBarrierHeight == 0)
{
ptw32_mcs_node_transfer(&b->proxynode, &node);
result = (b->nInitialBarrierHeight > 1
? sem_post_multiple (&(b->semBarrierBreeched),
b->nInitialBarrierHeight - 1) : 0);
}
else
{
ptw32_mcs_lock_release(&node);
result = ptw32_semwait (&(b->semBarrierBreeched));
}
if ((PTW32_INTERLOCKED_LONG)PTW32_INTERLOCKED_INCREMENT_LONG((PTW32_INTERLOCKED_LONGPTR)&b->nCurrentBarrierHeight)
== (PTW32_INTERLOCKED_LONG)b->nInitialBarrierHeight)
{
ptw32_mcs_lock_release(&b->proxynode);
if (0 == result)
{
result = PTHREAD_BARRIER_SERIAL_THREAD;
}
}
return (result);
}
