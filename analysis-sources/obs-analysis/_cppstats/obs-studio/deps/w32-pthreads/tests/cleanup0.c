#if defined(_MSC_VER) || defined(__cplusplus)
#include "test.h"
enum {
NUMTHREADS = 10
};
typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
int count;
};
static bag_t threadbag[NUMTHREADS + 1];
typedef struct {
int i;
CRITICAL_SECTION cs;
} sharedInt_t;
static sharedInt_t pop_count = {0, {0}};
static void
increment_pop_count(void * arg)
{
sharedInt_t * sI = (sharedInt_t *) arg;
EnterCriticalSection(&sI->cs);
sI->i++;
LeaveCriticalSection(&sI->cs);
}
void *
mythread(void * arg)
{
int result = 0;
bag_t * bag = (bag_t *) arg;
assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;
assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
#if defined(_MSC_VER)
#pragma inline_depth(0)
#endif
pthread_cleanup_push(increment_pop_count, (void *) &pop_count);
Sleep(100);
pthread_cleanup_pop(1);
#if defined(_MSC_VER)
#pragma inline_depth()
#endif
return (void *) (size_t)result;
}
int
main()
{
int failed = 0;
int i;
pthread_t t[NUMTHREADS + 1];
InitializeCriticalSection(&pop_count.cs);
assert((t[0] = pthread_self()).p != NULL);
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
}
Sleep(500);
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
fail = ((int)(size_t)result == (int) PTHREAD_CANCELED);
if (fail)
{
fprintf(stderr, "Thread %d: started %d: result %d\n",
i,
threadbag[i].started,
(int)(size_t)result);
fflush(stderr);
}
failed = (failed || fail);
}
assert(!failed);
assert(pop_count.i == NUMTHREADS);
DeleteCriticalSection(&pop_count.cs);
return 0;
}
#else 
int
main()
{
return 0;
}
#endif 
