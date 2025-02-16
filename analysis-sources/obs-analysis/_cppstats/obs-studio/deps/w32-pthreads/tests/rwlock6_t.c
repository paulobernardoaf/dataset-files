#include "test.h"
#include <sys/timeb.h>
static pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;
static int bankAccount = 0;
void * wrfunc(void * arg)
{
assert(pthread_rwlock_wrlock(&rwlock1) == 0);
Sleep(2000);
bankAccount += 10;
assert(pthread_rwlock_unlock(&rwlock1) == 0);
return ((void *)(size_t)bankAccount);
}
void * rdfunc(void * arg)
{
int ba = -1;
struct timespec abstime = { 0, 0 };
PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;
PTW32_FTIME(&currSysTime);
abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
if ((int) (size_t)arg == 1)
{
abstime.tv_sec += 1;
assert(pthread_rwlock_timedrdlock(&rwlock1, &abstime) == ETIMEDOUT);
ba = 0;
}
else if ((int) (size_t)arg == 2)
{
abstime.tv_sec += 3;
assert(pthread_rwlock_timedrdlock(&rwlock1, &abstime) == 0);
ba = bankAccount;
assert(pthread_rwlock_unlock(&rwlock1) == 0);
}
return ((void *)(size_t)ba);
}
int
main()
{
pthread_t wrt1;
pthread_t wrt2;
pthread_t rdt1;
pthread_t rdt2;
void* wr1Result = (void*)0;
void* wr2Result = (void*)0;
void* rd1Result = (void*)0;
void* rd2Result = (void*)0;
bankAccount = 0;
assert(pthread_create(&wrt1, NULL, wrfunc, NULL) == 0);
Sleep(500);
assert(pthread_create(&rdt1, NULL, rdfunc, (void *)(size_t)1) == 0);
Sleep(500);
assert(pthread_create(&wrt2, NULL, wrfunc, NULL) == 0);
Sleep(500);
assert(pthread_create(&rdt2, NULL, rdfunc, (void *)(size_t)2) == 0);
assert(pthread_join(wrt1, &wr1Result) == 0);
assert(pthread_join(rdt1, &rd1Result) == 0);
assert(pthread_join(wrt2, &wr2Result) == 0);
assert(pthread_join(rdt2, &rd2Result) == 0);
assert((int)(size_t)wr1Result == 10);
assert((int)(size_t)rd1Result == 0);
assert((int)(size_t)wr2Result == 20);
assert((int)(size_t)rd2Result == 20);
return 0;
}
