#include "pthread.h"
#include "implement.h"
int
pthread_mutexattr_getrobust (const pthread_mutexattr_t * attr, int * robust)
{
int result = EINVAL;
if ((attr != NULL && *attr != NULL && robust != NULL))
{
*robust = (*attr)->robustness;
result = 0;
}
return (result);
} 
