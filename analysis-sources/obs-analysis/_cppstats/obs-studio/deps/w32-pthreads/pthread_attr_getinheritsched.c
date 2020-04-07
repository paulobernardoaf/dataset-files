#include "pthread.h"
#include "implement.h"
#include "sched.h"
int
pthread_attr_getinheritsched (const pthread_attr_t * attr, int *inheritsched)
{
if (ptw32_is_attr (attr) != 0 || inheritsched == NULL)
{
return EINVAL;
}
*inheritsched = (*attr)->inheritsched;
return 0;
}
