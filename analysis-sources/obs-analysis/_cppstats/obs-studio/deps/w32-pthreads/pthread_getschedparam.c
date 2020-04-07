#include "pthread.h"
#include "implement.h"
#include "sched.h"
int
pthread_getschedparam (pthread_t thread, int *policy,
struct sched_param *param)
{
int result;
result = pthread_kill (thread, 0);
if (0 != result)
{
return result;
}
if (policy <= (int *) SCHED_MAX || param == NULL)
{
return EINVAL;
}
*policy = SCHED_OTHER;
param->sched_priority = ((ptw32_thread_t *)thread.p)->sched_priority;
return 0;
}
