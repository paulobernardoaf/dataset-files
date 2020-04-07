



































#include "pthread.h"
#include "implement.h"

int
pthread_once (pthread_once_t * once_control, void (PTW32_CDECL *init_routine) (void))
{
if (once_control == NULL || init_routine == NULL)
{
return EINVAL;
}

if ((PTW32_INTERLOCKED_LONG)PTW32_FALSE ==
(PTW32_INTERLOCKED_LONG)PTW32_INTERLOCKED_EXCHANGE_ADD_LONG((PTW32_INTERLOCKED_LONGPTR)&once_control->done,
(PTW32_INTERLOCKED_LONG)0)) 
{
ptw32_mcs_local_node_t node;

ptw32_mcs_lock_acquire((ptw32_mcs_lock_t *)&once_control->lock, &node);

if (!once_control->done)
{

#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth(0)
#endif

pthread_cleanup_push(ptw32_mcs_lock_release, &node);
(*init_routine)();
pthread_cleanup_pop(0);

#if defined(_MSC_VER) && _MSC_VER < 1400
#pragma inline_depth()
#endif

once_control->done = PTW32_TRUE;
}

ptw32_mcs_lock_release(&node);
}

return 0;

} 
