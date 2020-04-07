#include "pthread.h"
#include "implement.h"
int
pthread_attr_setstackaddr (pthread_attr_t * attr, void *stackaddr)
{
#if defined( _POSIX_THREAD_ATTR_STACKADDR )
if (ptw32_is_attr (attr) != 0)
{
return EINVAL;
}
(*attr)->stackaddr = stackaddr;
return 0;
#else
return ENOSYS;
#endif 
}
