#include "pthread.h"
#include "implement.h"
int
pthread_barrierattr_destroy (pthread_barrierattr_t * attr)
{
int result = 0;
if (attr == NULL || *attr == NULL)
{
result = EINVAL;
}
else
{
pthread_barrierattr_t ba = *attr;
*attr = NULL;
free (ba);
}
return (result);
} 
