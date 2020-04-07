#if defined(__cplusplus)
#include "test.h"
enum {
NUMTHREADS = 4
};
typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
};
static bag_t threadbag[NUMTHREADS + 1];
static pthread_mutex_t waitLock = PTHREAD_MUTEX_INITIALIZER;
void *
mythread(void * arg)
{
int result = 0;
bag_t * bag = (bag_t *) arg;
assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;
assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
switch (bag->threadnum % 2)
{
case 0:
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
result = 0;
break;
case 1:
assert(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0);
result = 1;
break;
}
#if !defined(__cplusplus)
__try
#else
try
#endif
{
assert(pthread_mutex_lock(&waitLock) == 0);
assert(pthread_mutex_unlock(&waitLock) == 0);
sched_yield();
for (;;)
{
pthread_testcancel();
}
}
#if !defined(__cplusplus)
__except(EXCEPTION_EXECUTE_HANDLER)
#else
#if defined(PtW32CatchAll)
PtW32CatchAll
#else
catch(...)
#endif
#endif
{
result += 100;
}
result += 1000;
return (void *) (size_t)result;
}
int
main()
{
int failed = 0;
int i;
pthread_t t[NUMTHREADS + 1];
assert((t[0] = pthread_self()).p != NULL);
assert(pthread_mutex_lock(&waitLock) == 0);
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
}
Sleep(500);
assert(pthread_mutex_unlock(&waitLock) == 0);
Sleep(500);
for (i = 1; i <= NUMTHREADS; i++)
{
assert(pthread_cancel(t[i]) == 0);
}
Sleep(NUMTHREADS * 100);
for (i = 1; i <= NUMTHREADS; i++)
{ 
if (!threadbag[i].started)
{
failed |= !threadbag[i].started;
fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
}
}
assert(!failed);
failed = 0;
for (i = 1; i <= NUMTHREADS; i++)
{
int fail = 0;
void* result = (void*)0;
assert(pthread_join(t[i], &result) == 0);
fail = ((int)(size_t)result != (int) PTHREAD_CANCELED);
if (fail)
{
fprintf(stderr, "Thread %d: started %d: location %d: cancel type %s\n",
i,
threadbag[i].started,
(int)(size_t)result,
(((int)(size_t)result % 2) == 0) ? "ASYNCHRONOUS" : "DEFERRED");
}
failed |= fail;
}
assert(!failed);
return 0;
}
#else 
#include <stdio.h>
int
main()
{
fprintf(stderr, "Test N/A for this compiler environment.\n");
return 0;
}
#endif 
