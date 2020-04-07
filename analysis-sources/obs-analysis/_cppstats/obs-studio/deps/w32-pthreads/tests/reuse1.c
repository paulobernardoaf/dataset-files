#include "test.h"
enum {
NUMTHREADS = 100
};
static int washere = 0;
void * func(void * arg)
{
washere = 1;
return arg; 
}
int
main()
{
pthread_t t,
last_t;
pthread_attr_t attr;
void * result = NULL;
int i;
assert(pthread_attr_init(&attr) == 0);;
assert(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0);
washere = 0;
assert(pthread_create(&t, &attr, func, NULL) == 0);
assert(pthread_join(t, &result) == 0);;
assert((int)(size_t)result == 0);
assert(washere == 1);
last_t = t;
for (i = 1; i < NUMTHREADS; i++)
{
washere = 0;
assert(pthread_create(&t, &attr, func, (void *)(size_t)i) == 0);
pthread_join(t, &result);
assert((int)(size_t) result == i);
assert(washere == 1);
assert(!pthread_equal(t, last_t));
assert(t.p == last_t.p);
assert(t.x == last_t.x+1);
last_t = t;
}
return 0;
}
