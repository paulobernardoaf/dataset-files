#include "pthread.h"
#include "implement.h"
int
pthread_mutexattr_init (pthread_mutexattr_t * attr)
{
int result = 0;
pthread_mutexattr_t ma;
ma = (pthread_mutexattr_t) calloc (1, sizeof (*ma));
if (ma == NULL)
{
result = ENOMEM;
}
else
{
ma->pshared = PTHREAD_PROCESS_PRIVATE;
ma->kind = PTHREAD_MUTEX_DEFAULT;
}
*attr = ma;
return (result);
} 
