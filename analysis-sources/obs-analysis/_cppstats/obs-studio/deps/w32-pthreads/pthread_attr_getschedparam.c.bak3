



































#include "pthread.h"
#include "implement.h"
#include "sched.h"

int
pthread_attr_getschedparam (const pthread_attr_t * attr,
struct sched_param *param)
{
if (ptw32_is_attr (attr) != 0 || param == NULL)
{
return EINVAL;
}

memcpy (param, &(*attr)->param, sizeof (*param));
return 0;
}
