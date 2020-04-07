#include "test.h"
#define NUMTHREADS (30)
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t threads[NUMTHREADS];
static unsigned numThreads = 0;
void *
myfunc(void *arg)
{
pthread_mutex_lock(&lock);
numThreads++;
pthread_mutex_unlock(&lock);
Sleep(1000);
return 0;
}
int
main()
{
int i;
int maxThreads = sizeof(threads) / sizeof(pthread_t);
for (i = 0; i < maxThreads; i++)
{
assert(pthread_create(&threads[i], NULL, myfunc, 0) == 0);
}
for (i = 0; i < maxThreads; i++)
{
assert(pthread_join(threads[i], NULL) == 0);
}
assert((int) numThreads == maxThreads);
return 0;
}
