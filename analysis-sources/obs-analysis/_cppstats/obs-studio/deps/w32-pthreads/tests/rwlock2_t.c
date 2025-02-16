#include "test.h"
#include <sys/timeb.h>
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int
main()
{
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
assert(rwlock == PTHREAD_RWLOCK_INITIALIZER);
assert(pthread_rwlock_timedrdlock(&rwlock, &abstime) == 0);
assert(rwlock != PTHREAD_RWLOCK_INITIALIZER);
assert(rwlock != NULL);
assert(pthread_rwlock_unlock(&rwlock) == 0);
assert(pthread_rwlock_destroy(&rwlock) == 0);
assert(rwlock == NULL);
return 0;
}
