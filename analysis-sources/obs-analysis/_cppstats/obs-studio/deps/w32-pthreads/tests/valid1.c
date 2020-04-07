#include "test.h"
enum {
NUMTHREADS = 1
};
static int washere = 0;
void * func(void * arg)
{
washere = 1;
return (void *) 0; 
}
int
main()
{
pthread_t t;
void * result = NULL;
washere = 0;
assert(pthread_create(&t, NULL, func, NULL) == 0);
assert(pthread_join(t, &result) == 0);
assert((int)(size_t)result == 0);
assert(washere == 1);
sched_yield();
assert(pthread_kill(t, 0) == ESRCH);
return 0;
}
