












































































#define _WIN32_WINNT 0x400

#include "test.h"
#include <sys/timeb.h>

static pthread_cond_t cv;
static pthread_mutex_t mutex;
static struct timespec abstime = { 0, 0 };
static struct timespec abstime2 = { 0, 0 };
static int timedout = 0;
static int awoken = 0;

enum {
NUMTHREADS = 30
};

void *
mythread(void * arg)
{
int result;

assert(pthread_mutex_lock(&mutex) == 0);

abstime2.tv_sec = abstime.tv_sec;

if ((int) (size_t)arg % 3 == 0)
{
abstime2.tv_sec += 2;
}

result = pthread_cond_timedwait(&cv, &mutex, &abstime2);
assert(pthread_mutex_unlock(&mutex) == 0);
if (result == ETIMEDOUT)
{
InterlockedIncrement((LPLONG)&timedout);
}
else
{
InterlockedIncrement((LPLONG)&awoken);
}


return arg;
}

#include "../implement.h"

int
main()
{
int i;
pthread_t t[NUMTHREADS + 1];
void* result = (void*)0;
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;

assert(pthread_cond_init(&cv, NULL) == 0);

assert(pthread_mutex_init(&mutex, NULL) == 0);


PTW32_FTIME(&currSysTime);

abstime.tv_sec = abstime2.tv_sec = (long)currSysTime.time + 5;
abstime.tv_nsec = abstime2.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

assert(pthread_mutex_lock(&mutex) == 0);

for (i = 1; i <= NUMTHREADS; i++)
{
assert(pthread_create(&t[i], NULL, mythread, (void *)(size_t)i) == 0);
}

assert(pthread_mutex_unlock(&mutex) == 0);

for (i = 1; i <= NUMTHREADS; i++)
{
assert(pthread_join(t[i], &result) == 0);
assert((int)(size_t)result == i);









if (InterlockedExchangeAdd((LPLONG)&awoken, 0L) > NUMTHREADS/3)
{
assert(pthread_cond_broadcast(&cv) == 0);
}



}

assert(awoken == NUMTHREADS - timedout);

{
int result = pthread_cond_destroy(&cv);
if (result != 0)
{
fprintf(stderr, "Result = %s\n", error_string[result]);
fprintf(stderr, "\tWaitersBlocked = %ld\n", cv->nWaitersBlocked);
fprintf(stderr, "\tWaitersGone = %ld\n", cv->nWaitersGone);
fprintf(stderr, "\tWaitersToUnblock = %ld\n", cv->nWaitersToUnblock);
fflush(stderr);
}
assert(result == 0);
}

assert(pthread_mutex_destroy(&mutex) == 0);

return 0;
}
