











































































#include "test.h"




enum {
NUMTHREADS = 10000
};


static long done = 0;






static UINT64 seqmap[NUMTHREADS+2];

void * func(void * arg)
{
sched_yield();
seqmap[(int)pthread_getunique_np(pthread_self())] = 1;
InterlockedIncrement(&done);

return (void *) 0; 
}

int
main()
{
pthread_t t[NUMTHREADS];
pthread_attr_t attr;
int i;

assert(pthread_attr_init(&attr) == 0);
assert(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) == 0);

for (i = 0; i < NUMTHREADS+2; i++)
{
seqmap[i] = 0;
}

for (i = 0; i < NUMTHREADS; i++)
{
if (NUMTHREADS/2 == i)
{

seqmap[(int)pthread_getunique_np(pthread_self())] = 1;
}
assert(pthread_create(&t[i], &attr, func, NULL) == 0);
}

while (NUMTHREADS > InterlockedExchangeAdd((LPLONG)&done, 0L))
Sleep(100);

Sleep(100);

assert(seqmap[0] == 0);
for (i = 1; i < NUMTHREADS+2; i++)
{
assert(seqmap[i] == 1);
}

return 0;
}
