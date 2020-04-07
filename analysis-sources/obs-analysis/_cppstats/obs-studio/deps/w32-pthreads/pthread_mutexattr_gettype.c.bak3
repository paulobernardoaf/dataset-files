



































#include "pthread.h"
#include "implement.h"


int
pthread_mutexattr_gettype (const pthread_mutexattr_t * attr, int *kind)
{
int result = 0;

if (attr != NULL && *attr != NULL && kind != NULL)
{
*kind = (*attr)->kind;
}
else
{
result = EINVAL;
}

return (result);
}
