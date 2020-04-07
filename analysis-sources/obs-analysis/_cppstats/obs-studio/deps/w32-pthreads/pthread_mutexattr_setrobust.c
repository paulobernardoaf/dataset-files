#include "pthread.h"
#include "implement.h"
int
pthread_mutexattr_setrobust (pthread_mutexattr_t * attr, int robust)
{
int result = EINVAL;
if ((attr != NULL && *attr != NULL))
{
switch (robust)
{
case PTHREAD_MUTEX_STALLED:
case PTHREAD_MUTEX_ROBUST:
(*attr)->robustness = robust;
result = 0;
break;
}
}
return (result);
} 
