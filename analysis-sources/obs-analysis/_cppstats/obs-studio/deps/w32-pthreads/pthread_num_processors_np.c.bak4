



































#include "pthread.h"
#include "implement.h"






int
pthread_num_processors_np (void)
{
int count;

if (ptw32_getprocessors (&count) != 0)
{
count = 1;
}

return (count);
}
