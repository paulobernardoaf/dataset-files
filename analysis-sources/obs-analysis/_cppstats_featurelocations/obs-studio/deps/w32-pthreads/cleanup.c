





































#include "pthread.h"
#include "implement.h"








ptw32_cleanup_t *
ptw32_pop_cleanup (int execute)
























{
ptw32_cleanup_t *cleanup;

cleanup = (ptw32_cleanup_t *) pthread_getspecific (ptw32_cleanupKey);

if (cleanup != NULL)
{
if (execute && (cleanup->routine != NULL))
{

(*cleanup->routine) (cleanup->arg);

}

pthread_setspecific (ptw32_cleanupKey, (void *) cleanup->prev);

}

return (cleanup);

} 


void
ptw32_push_cleanup (ptw32_cleanup_t * cleanup,
ptw32_cleanup_callback_t routine, void *arg)







































{
cleanup->routine = routine;
cleanup->arg = arg;

cleanup->prev = (ptw32_cleanup_t *) pthread_getspecific (ptw32_cleanupKey);

pthread_setspecific (ptw32_cleanupKey, (void *) cleanup);

} 
