



































#include <limits.h>

#include "pthread.h"
#include "implement.h"

int
pthread_rwlockattr_destroy (pthread_rwlockattr_t * attr)
























{
int result = 0;

if (attr == NULL || *attr == NULL)
{
result = EINVAL;
}
else
{
pthread_rwlockattr_t rwa = *attr;

*attr = NULL;
free (rwa);
}

return (result);
} 
