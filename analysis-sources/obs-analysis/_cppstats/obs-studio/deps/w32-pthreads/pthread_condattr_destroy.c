#include "pthread.h"
#include "implement.h"
int
pthread_condattr_destroy (pthread_condattr_t * attr)
{
int result = 0;
if (attr == NULL || *attr == NULL)
{
result = EINVAL;
}
else
{
(void) free (*attr);
*attr = NULL;
result = 0;
}
return result;
} 
