#include "pthread.h"
#include "implement.h"
#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif
int
pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * stacksize)
{
#if defined(_POSIX_THREAD_ATTR_STACKSIZE)
if (ptw32_is_attr (attr) != 0)
{
return EINVAL;
}
*stacksize = (*attr)->stacksize;
return 0;
#else
return ENOSYS;
#endif 
}
