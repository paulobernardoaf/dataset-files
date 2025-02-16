#include "pthread.h"
#include "implement.h"
#include "sched.h"
int
pthread_attr_setschedparam (pthread_attr_t * attr,
const struct sched_param *param)
{
int priority;
if (ptw32_is_attr (attr) != 0 || param == NULL)
{
return EINVAL;
}
priority = param->sched_priority;
if (priority < sched_get_priority_min (SCHED_OTHER) ||
priority > sched_get_priority_max (SCHED_OTHER))
{
return EINVAL;
}
memcpy (&(*attr)->param, param, sizeof (*param));
return 0;
}
