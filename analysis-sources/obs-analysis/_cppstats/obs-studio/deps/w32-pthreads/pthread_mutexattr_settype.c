#include "pthread.h"
#include "implement.h"
int
pthread_mutexattr_settype (pthread_mutexattr_t * attr, int kind)
{
int result = 0;
if ((attr != NULL && *attr != NULL))
{
switch (kind)
{
case PTHREAD_MUTEX_FAST_NP:
case PTHREAD_MUTEX_RECURSIVE_NP:
case PTHREAD_MUTEX_ERRORCHECK_NP:
(*attr)->kind = kind;
break;
default:
result = EINVAL;
break;
}
}
else
{
result = EINVAL;
}
return (result);
} 
