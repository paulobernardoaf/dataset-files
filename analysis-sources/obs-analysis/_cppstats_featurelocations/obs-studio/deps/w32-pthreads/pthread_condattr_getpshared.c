




































#include "pthread.h"
#include "implement.h"


int
pthread_condattr_getpshared (const pthread_condattr_t * attr, int *pshared)






































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

return result;

} 
