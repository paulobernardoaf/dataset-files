#include <limits.h>
#include "pthread.h"
#include "implement.h"
int
pthread_rwlockattr_init (pthread_rwlockattr_t * attr)
{
int result = 0;
pthread_rwlockattr_t rwa;
rwa = (pthread_rwlockattr_t) calloc (1, sizeof (*rwa));
if (rwa == NULL)
{
result = ENOMEM;
}
else
{
rwa->pshared = PTHREAD_PROCESS_PRIVATE;
}
*attr = rwa;
return (result);
} 
