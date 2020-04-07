



































#include "pthread.h"
#include "implement.h"


#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif

int
pthread_attr_setscope (pthread_attr_t * attr, int contentionscope)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
switch (contentionscope)
{
case PTHREAD_SCOPE_SYSTEM:
(*attr)->contentionscope = contentionscope;
return 0;
case PTHREAD_SCOPE_PROCESS:
return ENOTSUP;
default:
return EINVAL;
}
#else
return ENOSYS;
#endif
}
