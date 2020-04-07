#include "test.h"
#include <sys/timeb.h>
pthread_cond_t cnd;
pthread_mutex_t mtx;
int main()
{
int rc;
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
assert(pthread_cond_init(&cnd, 0) == 0);
assert(pthread_mutex_init(&mtx, 0) == 0);
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
assert(pthread_mutex_lock(&mtx) == 0);
assert((rc = pthread_cond_timedwait(&cnd, &mtx, &abstime)) == ETIMEDOUT);
assert(pthread_mutex_unlock(&mtx) == 0);
assert((rc = pthread_cond_signal(&cnd)) == 0);
assert(pthread_mutex_lock(&mtx) == 0);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
assert((rc = pthread_cond_timedwait(&cnd, &mtx, &abstime)) == ETIMEDOUT);
assert(pthread_mutex_unlock(&mtx) == 0);
return 0;
}
