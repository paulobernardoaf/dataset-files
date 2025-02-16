



































#include "pthread.h"
#include "implement.h"


int
pthread_attr_init (pthread_attr_t * attr)
























{
pthread_attr_t attr_result;

if (attr == NULL)
{

return EINVAL;
}

attr_result = (pthread_attr_t) malloc (sizeof (*attr_result));

if (attr_result == NULL)
{
return ENOMEM;
}

#if defined(_POSIX_THREAD_ATTR_STACKSIZE)





attr_result->stacksize = 0;
#endif

#if defined(_POSIX_THREAD_ATTR_STACKADDR)

attr_result->stackaddr = NULL;
#endif

attr_result->detachstate = PTHREAD_CREATE_JOINABLE;

#if defined(HAVE_SIGSET_T)
memset (&(attr_result->sigmask), 0, sizeof (sigset_t));
#endif 






attr_result->param.sched_priority = THREAD_PRIORITY_NORMAL;
attr_result->inheritsched = PTHREAD_EXPLICIT_SCHED;
attr_result->contentionscope = PTHREAD_SCOPE_SYSTEM;

attr_result->valid = PTW32_ATTR_VALID;

*attr = attr_result;

return 0;
}
