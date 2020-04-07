












































































#define _WIN32_WINNT 0x400

#include "test.h"
#include <sys/timeb.h>

static pthread_cond_t cv;
static pthread_cond_t cv1;
static pthread_mutex_t mutex;
static pthread_mutex_t mutex1;
static struct timespec abstime = { 0, 0 };
static int timedout = 0;
static int signaled = 0;
static int awoken = 0;
static int waiting = 0;

enum {
NUMTHREADS = 30
};

void *
mythread(void * arg)
{
int result;

assert(pthread_mutex_lock(&mutex1) == 0);
++waiting;
assert(pthread_mutex_unlock(&mutex1) == 0);
assert(pthread_cond_signal(&cv1) == 0);

assert(pthread_mutex_lock(&mutex) == 0);
result = pthread_cond_timedwait(&cv, &mutex, &abstime);
if (result == ETIMEDOUT)
{
timedout++;
}
else
{
awoken++;
}
assert(pthread_mutex_unlock(&mutex) == 0);

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
assert(pthread_cond_init(&cv1, NULL) == 0);

assert(pthread_mutex_init(&mutex, NULL) == 0);
assert(pthread_mutex_init(&mutex1, NULL) == 0);


PTW32_FTIME(&currSysTime);

abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

abstime.tv_sec += 5;

assert(pthread_mutex_lock(&mutex1) == 0);

for (i = 1; i <= NUMTHREADS; i++)
{
assert(pthread_create(&t[i], NULL, mythread, (void *)(size_t)i) == 0);
}

do {
assert(pthread_cond_wait(&cv1,&mutex1) == 0);
} while ( NUMTHREADS > waiting );

assert(pthread_mutex_unlock(&mutex1) == 0);

for (i = NUMTHREADS/3; i <= 2*NUMTHREADS/3; i++)
{

assert(pthread_cond_signal(&cv) == 0);


signaled++;
}

for (i = 1; i <= NUMTHREADS; i++)
{
assert(pthread_join(t[i], &result) == 0);
assert((int)(size_t)result == i);
}

fprintf(stderr, "awk = %d\n", awoken);
fprintf(stderr, "sig = %d\n", signaled);
fprintf(stderr, "tot = %d\n", timedout);

assert(signaled == awoken);
assert(timedout == NUMTHREADS - signaled);

assert(pthread_cond_destroy(&cv1) == 0);

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

assert(pthread_mutex_destroy(&mutex1) == 0);
assert(pthread_mutex_destroy(&mutex) == 0);

return 0;
}
