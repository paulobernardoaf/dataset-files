#include "pthread.h"
#include "implement.h"
int
pthread_barrierattr_getpshared (const pthread_barrierattr_t * attr,
int *pshared)
{
int result;
if ((attr != NULL && *attr != NULL) && (pshared != NULL))
{
*pshared = (*attr)->pshared;
result = 0;
}
else
{
result = EINVAL;
}
return (result);
} 
