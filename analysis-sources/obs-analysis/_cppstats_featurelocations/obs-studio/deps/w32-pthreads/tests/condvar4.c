











































































#include "test.h"
#include <sys/timeb.h>

typedef struct cvthing_t_ cvthing_t;

struct cvthing_t_ {
pthread_cond_t notbusy;
pthread_mutex_t lock;
int shared;
};

static cvthing_t cvthing = {
PTHREAD_COND_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
0
};

enum {
NUMTHREADS = 2
};

void *
mythread(void * arg)
{
assert(pthread_mutex_lock(&cvthing.lock) == 0);
cvthing.shared++;
assert(pthread_mutex_unlock(&cvthing.lock) == 0);

assert(pthread_cond_signal(&cvthing.notbusy) == 0);

return (void *) 0;
}

int
main()
{
pthread_t t[NUMTHREADS];
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;

cvthing.shared = 0;

assert((t[0] = pthread_self()).p != NULL);

assert(cvthing.notbusy == PTHREAD_COND_INITIALIZER);

assert(cvthing.lock == PTHREAD_MUTEX_INITIALIZER);

assert(pthread_mutex_lock(&cvthing.lock) == 0);

assert(cvthing.lock != PTHREAD_MUTEX_INITIALIZER);


PTW32_FTIME(&currSysTime);

abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

abstime.tv_sec += 5;

assert(pthread_cond_timedwait(&cvthing.notbusy, &cvthing.lock, &abstime) == ETIMEDOUT);

assert(cvthing.notbusy != PTHREAD_COND_INITIALIZER);

assert(pthread_create(&t[1], NULL, mythread, (void *) 1) == 0);

PTW32_FTIME(&currSysTime);

abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

abstime.tv_sec += 5;

while (! (cvthing.shared > 0))
assert(pthread_cond_timedwait(&cvthing.notbusy, &cvthing.lock, &abstime) == 0);

assert(cvthing.shared > 0);

assert(pthread_mutex_unlock(&cvthing.lock) == 0);

assert(pthread_join(t[1], NULL) == 0);

assert(pthread_mutex_destroy(&cvthing.lock) == 0);

assert(cvthing.lock == NULL);

assert(pthread_cond_destroy(&cvthing.notbusy) == 0);

assert(cvthing.notbusy == NULL);

return 0;
}
