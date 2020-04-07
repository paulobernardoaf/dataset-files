#include "pthread.h"
#include "implement.h"
int
pthread_mutex_destroy (pthread_mutex_t * mutex)
{
int result = 0;
pthread_mutex_t mx;
if (*mutex < PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
{
mx = *mutex;
result = pthread_mutex_trylock (&mx);
if (0 == result || ENOTRECOVERABLE == result)
{
if (mx->kind != PTHREAD_MUTEX_RECURSIVE || 1 == mx->recursive_count)
{
*mutex = NULL;
result = (0 == result)?pthread_mutex_unlock(&mx):0;
if (0 == result)
{
if (mx->robustNode != NULL)
{
free(mx->robustNode);
}
if (!CloseHandle (mx->event))
{
*mutex = mx;
result = EINVAL;
}
else
{
free (mx);
}
}
else
{
*mutex = mx;
}
}
else 
{
mx->recursive_count--; 
result = EBUSY;
}
}
}
else
{
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_mutex_test_init_lock, &node);
if (*mutex >= PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
{
*mutex = NULL;
}
else
{
result = EBUSY;
}
ptw32_mcs_lock_release(&node);
}
return (result);
}
