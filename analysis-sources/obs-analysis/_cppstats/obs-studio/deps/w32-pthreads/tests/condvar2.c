#define _WIN32_WINNT 0x400
#include "test.h"
#include <sys/timeb.h>
pthread_cond_t cv;
pthread_mutex_t mutex;
#include "../implement.h"
int
main()
{
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
assert(pthread_cond_init(&cv, NULL) == 0);
assert(pthread_mutex_init(&mutex, NULL) == 0);
assert(pthread_mutex_lock(&mutex) == 0);
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
assert(pthread_cond_timedwait(&cv, &mutex, &abstime) == ETIMEDOUT);
assert(pthread_mutex_unlock(&mutex) == 0);
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
return 0;
}
