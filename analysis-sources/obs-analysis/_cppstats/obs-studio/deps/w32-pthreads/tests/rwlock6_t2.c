#include "test.h"
#include <sys/timeb.h>
static pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;
static int bankAccount = 0;
struct timespec abstime = { 0, 0 };
void * wrfunc(void * arg)
{
int result;
result = pthread_rwlock_timedwrlock(&rwlock1, &abstime);
if ((int) (size_t)arg == 1)
{
assert(result == 0);
Sleep(2000);
bankAccount += 10;
assert(pthread_rwlock_unlock(&rwlock1) == 0);
return ((void *)(size_t)bankAccount);
}
else if ((int) (size_t)arg == 2)
{
assert(result == ETIMEDOUT);
return ((void *) 100);
}
return ((void *)(size_t)-1);
}
void * rdfunc(void * arg)
{
int ba = 0;
assert(pthread_rwlock_timedrdlock(&rwlock1, &abstime) == ETIMEDOUT);
return ((void *)(size_t)ba);
}
int
main()
{
pthread_t wrt1;
pthread_t wrt2;
pthread_t rdt;
void* wr1Result = (void*)0;
void* wr2Result = (void*)0;
void* rdResult = (void*)0;
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
abstime.tv_sec += 1;
bankAccount = 0;
assert(pthread_create(&wrt1, NULL, wrfunc, (void *)(size_t)1) == 0);
Sleep(100);
assert(pthread_create(&rdt, NULL, rdfunc, NULL) == 0);
Sleep(100);
assert(pthread_create(&wrt2, NULL, wrfunc, (void *)(size_t)2) == 0);
assert(pthread_join(wrt1, &wr1Result) == 0);
assert(pthread_join(rdt, &rdResult) == 0);
assert(pthread_join(wrt2, &wr2Result) == 0);
assert((int)(size_t)wr1Result == 10);
assert((int)(size_t)rdResult == 0);
assert((int)(size_t)wr2Result == 100);
return 0;
}
