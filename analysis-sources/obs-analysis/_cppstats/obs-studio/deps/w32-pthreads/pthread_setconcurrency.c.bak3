



































#include "pthread.h"
#include "implement.h"


int
pthread_setconcurrency (int level)
{
if (level < 0)
{
return EINVAL;
}
else
{
ptw32_concurrency = level;
return 0;
}
}
