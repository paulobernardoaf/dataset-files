



































#include "pthread.h"
#include "implement.h"


#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif

int
pthread_attr_getscope (const pthread_attr_t * attr, int *contentionscope)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
*contentionscope = (*attr)->contentionscope;
return 0;
#else
return ENOSYS;
#endif
}
