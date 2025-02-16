#include "test.h"
enum {
NUMTHREADS = 10000
};
static long done = 0;
void * func(void * arg)
{
sched_yield();
InterlockedIncrement(&done);
return (void *) 0; 
}
int
main()
{
pthread_t t[NUMTHREADS];
pthread_attr_t attr;
int i;
unsigned int notUnique = 0,
totalHandles = 0,
reuseMax = 0,
reuseMin = NUMTHREADS;
assert(pthread_attr_init(&attr) == 0);
assert(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);
for (i = 0; i < NUMTHREADS; i++)
{
while(pthread_create(&t[i], &attr, func, NULL) != 0)
Sleep(1);
}
while (NUMTHREADS > InterlockedExchangeAdd((LPLONG)&done, 0L))
Sleep(100);
Sleep(100);
for (i = 0; i < NUMTHREADS; i++)
{
if (t[i].p != NULL)
{
unsigned int j, thisMax;
thisMax = t[i].x;
for (j = i+1; j < NUMTHREADS; j++)
if (t[i].p == t[j].p)
{
if (t[i].x == t[j].x)
notUnique++;
if (thisMax < t[j].x)
thisMax = t[j].x;
t[j].p = NULL;
}
if (reuseMin > thisMax)
reuseMin = thisMax;
if (reuseMax < thisMax)
reuseMax = thisMax;
}
}
for (i = 0; i < NUMTHREADS; i++)
if (t[i].p != NULL)
totalHandles++;
printf("For %d total threads:\n", NUMTHREADS);
printf("Non-unique IDs = %d\n", notUnique);
printf("Reuse maximum = %d\n", reuseMax + 1);
printf("Reuse minimum = %d\n", reuseMin + 1);
printf("Total handles = %d\n", totalHandles);
return 0;
}
