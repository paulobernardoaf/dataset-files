



































#include "pthread.h"
#include "implement.h"


int
pthread_attr_setstacksize (pthread_attr_t * attr, size_t stacksize)







































{
#if defined(_POSIX_THREAD_ATTR_STACKSIZE)

#if PTHREAD_STACK_MIN > 0


if (stacksize < PTHREAD_STACK_MIN)
{
return EINVAL;
}

#endif

if (ptw32_is_attr (attr) != 0)
{
return EINVAL;
}


(*attr)->stacksize = stacksize;
return 0;

#else

return ENOSYS;

#endif 

}
