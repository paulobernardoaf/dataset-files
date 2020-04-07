



































#include "pthread.h"
#include "implement.h"


int
pthread_barrierattr_setpshared (pthread_barrierattr_t * attr, int pshared)








































{
int result;

if ((attr != NULL && *attr != NULL) &&
((pshared == PTHREAD_PROCESS_SHARED) ||
(pshared == PTHREAD_PROCESS_PRIVATE)))
{
if (pshared == PTHREAD_PROCESS_SHARED)
{

#if !defined( _POSIX_THREAD_PROCESS_SHARED )

result = ENOSYS;
pshared = PTHREAD_PROCESS_PRIVATE;

#else

result = 0;

#endif 

}
else
{
result = 0;
}

(*attr)->pshared = pshared;
}
else
{
result = EINVAL;
}

return (result);

} 
