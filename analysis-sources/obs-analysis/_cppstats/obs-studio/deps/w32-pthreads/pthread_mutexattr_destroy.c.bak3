



































#include "pthread.h"
#include "implement.h"


int
pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
























{
int result = 0;

if (attr == NULL || *attr == NULL)
{
result = EINVAL;
}
else
{
pthread_mutexattr_t ma = *attr;

*attr = NULL;
free (ma);
}

return (result);
} 
