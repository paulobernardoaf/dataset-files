#include "test.h"
#include <sys/timeb.h>
pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;
static int washere = 0;
void * func(void * arg)
{
assert(pthread_rwlock_trywrlock(&rwlock1) == EBUSY);
washere = 1;
return 0; 
}
int
main()
{
pthread_t t;
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
assert(pthread_rwlock_timedrdlock(&rwlock1, &abstime) == 0);
assert(pthread_create(&t, NULL, func, NULL) == 0);
Sleep(2000);
assert(pthread_rwlock_unlock(&rwlock1) == 0);
assert(washere == 1);
return 0;
}
