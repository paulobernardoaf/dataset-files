



































#include "pthread.h"
#include "implement.h"


#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif

int
pthread_attr_getstackaddr (const pthread_attr_t * attr, void **stackaddr)


































{
#if defined( _POSIX_THREAD_ATTR_STACKADDR )

if (ptw32_is_attr (attr) != 0)
{
return EINVAL;
}

*stackaddr = (*attr)->stackaddr;
return 0;

#else

return ENOSYS;

#endif 
}
