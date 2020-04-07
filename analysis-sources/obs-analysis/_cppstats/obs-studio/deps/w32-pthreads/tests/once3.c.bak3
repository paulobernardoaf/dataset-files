
















































#include "test.h"

#define NUM_THREADS 100 
#define NUM_ONCE 10

static pthread_once_t o = PTHREAD_ONCE_INIT;
static pthread_once_t once[NUM_ONCE];

typedef struct {
int i;
CRITICAL_SECTION cs;
} sharedInt_t;

static sharedInt_t numOnce = {0, {0}};
static sharedInt_t numThreads = {0, {0}};

void
myfunc(void)
{
EnterCriticalSection(&numOnce.cs);
numOnce.i++;
assert(numOnce.i > 0);
LeaveCriticalSection(&numOnce.cs);

Sleep(10);

pthread_testcancel();
}

void *
mythread(void * arg)
{






assert(pthread_cancel(pthread_self()) == 0);
assert(pthread_once(&once[(int)(size_t)arg], myfunc) == 0);
EnterCriticalSection(&numThreads.cs);
numThreads.i++;
LeaveCriticalSection(&numThreads.cs);
return (void*)(size_t)0;
}

int
main()
{
pthread_t t[NUM_THREADS][NUM_ONCE];
int i, j;

InitializeCriticalSection(&numThreads.cs);
InitializeCriticalSection(&numOnce.cs);

for (j = 0; j < NUM_ONCE; j++)
{
once[j] = o;

for (i = 0; i < NUM_THREADS; i++)
{

while (0 != pthread_create(&t[i][j], NULL, mythread, (void *)(size_t)j))
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

return 0;
}
