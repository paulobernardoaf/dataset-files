



































#include "pthread.h"
#include "implement.h"
#include "sched.h"

int
pthread_attr_setschedpolicy (pthread_attr_t * attr, int policy)
{
if (ptw32_is_attr (attr) != 0)
{
return EINVAL;
}

if (policy != SCHED_OTHER)
{
return ENOTSUP;
}

return 0;
}
