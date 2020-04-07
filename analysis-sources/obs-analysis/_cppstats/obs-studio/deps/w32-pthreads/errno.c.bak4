




































#if defined(NEED_ERRNO)

#include "pthread.h"
#include "implement.h"

static int reallyBad = ENOMEM;



























int *
_errno (void)
{
pthread_t self;
int *result;

if ((self = pthread_self ()).p == NULL)
{




result = &reallyBad;
}
else
{
result = (int *)(&self.p->exitStatus);
}

return (result);

} 

#endif 
