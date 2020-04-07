#include "test.h"
enum {
NUMTHREADS = 2
};
typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
};
static bag_t threadbag[NUMTHREADS + 1];
void *
mythread(void * arg)
{
bag_t * bag = (bag_t *) arg;
assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;
{
int oldstate;
int oldtype;
assert(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate) == 0);
assert(oldstate == PTHREAD_CANCEL_ENABLE); 
assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
assert(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) == 0);
assert(pthread_setcancelstate(oldstate, &oldstate) == 0);
assert(oldstate == PTHREAD_CANCEL_DISABLE); 
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype) == 0);
assert(oldtype == PTHREAD_CANCEL_DEFERRED); 
assert(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0);
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
assert(pthread_setcanceltype(oldtype, &oldtype) == 0);
assert(oldtype == PTHREAD_CANCEL_ASYNCHRONOUS); 
}
return 0;
}
int
main()
{
int failed = 0;
int i;
pthread_t t[NUMTHREADS + 1];
assert((t[0] = pthread_self()).p != NULL);
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
}
Sleep(NUMTHREADS * 100);
for (i = 1; i <= NUMTHREADS; i++)
{ 
failed = !threadbag[i].started;
if (failed)
{
fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
}
}
assert(!failed);
for (i = 1; i <= NUMTHREADS; i++)
{ 
}
assert(!failed);
return 0;
}
