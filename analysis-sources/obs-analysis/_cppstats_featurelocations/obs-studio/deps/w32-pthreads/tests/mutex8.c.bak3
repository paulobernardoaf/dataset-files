



































#include "test.h"
#include <sys/timeb.h>

static int lockCount = 0;

static pthread_mutex_t mutex;

void * locker(void * arg)
{
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;

PTW32_FTIME(&currSysTime);

abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

abstime.tv_sec += 1;

assert(pthread_mutex_timedlock(&mutex, &abstime) == ETIMEDOUT);

lockCount++;

return 0;
}

int
main()
{
pthread_t t;

assert(pthread_mutex_init(&mutex, NULL) == 0);

assert(pthread_mutex_lock(&mutex) == 0);

assert(pthread_create(&t, NULL, locker, NULL) == 0);

Sleep(2000);

assert(lockCount == 1);

assert(pthread_mutex_unlock(&mutex) == 0);

return 0;
}
