



































#include "pthread.h"
#include "implement.h"


int
pthread_barrierattr_init (pthread_barrierattr_t * attr)
























{
pthread_barrierattr_t ba;
int result = 0;

ba = (pthread_barrierattr_t) calloc (1, sizeof (*ba));

if (ba == NULL)
{
result = ENOMEM;
}
else
{
ba->pshared = PTHREAD_PROCESS_PRIVATE;
}

*attr = ba;

return (result);
} 
