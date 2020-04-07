














































#include "test.h"

#define NUM_THREADS 100 
#define NUM_ONCE 10

pthread_once_t o = PTHREAD_ONCE_INIT;
pthread_once_t once[NUM_ONCE];

typedef struct {
int i;
CRITICAL_SECTION cs;
} sharedInt_t;

static sharedInt_t numOnce = {0, {0}};
static sharedInt_t numThreads = {0, {0}};

typedef struct {
int threadnum;
int oncenum;
int myPrio;
HANDLE w32Thread;
} bag_t;

static bag_t threadbag[NUM_THREADS][NUM_ONCE];

CRITICAL_SECTION print_lock;

void
mycleanupfunc(void * arg)
{
bag_t * bag = (bag_t *) arg;
EnterCriticalSection(&print_lock);

printf("%4d %4d %4d %4d\n",
bag->oncenum,
bag->threadnum,
bag->myPrio,
bag->myPrio - GetThreadPriority(bag->w32Thread));
LeaveCriticalSection(&print_lock);
}

void
myinitfunc(void)
{
EnterCriticalSection(&numOnce.cs);
numOnce.i++;
LeaveCriticalSection(&numOnce.cs);

Sleep(10);

pthread_testcancel();
}

void *
mythread(void * arg)
{
bag_t * bag = (bag_t *) arg;
struct sched_param param;







pthread_t self = pthread_self();
bag->w32Thread = pthread_getw32threadhandle_np(self);



bag->myPrio = (bag->threadnum % 5) - 2;
param.sched_priority = bag->myPrio;
pthread_setschedparam(self, SCHED_OTHER, &param);


pthread_cancel(self);
#if 0
pthread_cleanup_push(mycleanupfunc, arg);
assert(pthread_once(&once[bag->oncenum], myinitfunc) == 0);
pthread_cleanup_pop(1);
#else
assert(pthread_once(&once[bag->oncenum], myinitfunc) == 0);
#endif
EnterCriticalSection(&numThreads.cs);
numThreads.i++;
LeaveCriticalSection(&numThreads.cs);
return 0;
}

int
main()
{
pthread_t t[NUM_THREADS][NUM_ONCE];
int i, j;

InitializeCriticalSection(&print_lock);
InitializeCriticalSection(&numThreads.cs);
InitializeCriticalSection(&numOnce.cs);

#if 0

printf("once thrd prio error\n");
#endif





SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

SetThreadPriority(GetCurrentThread(), -2);

for (j = 0; j < NUM_ONCE; j++)
{
once[j] = o;

for (i = 0; i < NUM_THREADS; i++)
{
bag_t * bag = &threadbag[i][j];
bag->threadnum = i;
bag->oncenum = j;

while (0 != pthread_create(&t[i][j], NULL, mythread, (void *)bag))
sched_yield();
}
}

for (j = 0; j < NUM_ONCE; j++)
for (i = 0; i < NUM_THREADS; i++)
if (pthread_join(t[i][j], NULL) != 0)
printf("Join failed for [thread,once] = [%d,%d]\n", i, j);






assert(numOnce.i == NUM_ONCE * NUM_THREADS);
assert(numThreads.i == 0);

DeleteCriticalSection(&numOnce.cs);
DeleteCriticalSection(&numThreads.cs);
DeleteCriticalSection(&print_lock);

return 0;
}
