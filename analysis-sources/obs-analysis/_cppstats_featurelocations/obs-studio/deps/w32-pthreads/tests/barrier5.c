







































#include "test.h"

enum {
NUMTHREADS = 15,
HEIGHT = 10,
BARRIERMULTIPLE = 1000
};

pthread_barrier_t barrier = NULL;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
LONG totalThreadCrossings;

void *
func(void * crossings)
{
int result;
int serialThreads = 0;

while ((LONG)(size_t)crossings >= (LONG)InterlockedIncrement((LPLONG)&totalThreadCrossings))
{
result = pthread_barrier_wait(&barrier);

if (result == PTHREAD_BARRIER_SERIAL_THREAD)
{
serialThreads++;
}
else if (result != 0)
{
printf("Barrier failed: result = %s\n", error_string[result]);
fflush(stdout);
return NULL;
}
}

return (void*)(size_t)serialThreads;
}

int
main()
{
int i, j;
void* result;
int serialThreadsTotal;
LONG Crossings;
pthread_t t[NUMTHREADS + 1];

for (j = 1; j <= NUMTHREADS; j++)
{
int height = j<HEIGHT?j:HEIGHT;

totalThreadCrossings = 0;
Crossings = height * BARRIERMULTIPLE;

printf("Threads=%d, Barrier height=%d\n", j, height);

assert(pthread_barrier_init(&barrier, NULL, height) == 0);

for (i = 1; i <= j; i++)
{
assert(pthread_create(&t[i], NULL, func, (void *)(size_t)Crossings) == 0);
}

serialThreadsTotal = 0;
for (i = 1; i <= j; i++)
{
assert(pthread_join(t[i], &result) == 0);
serialThreadsTotal += (int)(size_t)result;
}

assert(serialThreadsTotal == BARRIERMULTIPLE);

assert(pthread_barrier_destroy(&barrier) == 0);
}

assert(pthread_mutex_destroy(&mx) == 0);

return 0;
}
