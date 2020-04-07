#include "test.h"
#include <sys/timeb.h>
pthread_spinlock_t lock = PTHREAD_SPINLOCK_INITIALIZER;
PTW32_STRUCT_TIMEB currSysTimeStart;
PTW32_STRUCT_TIMEB currSysTimeStop;
#define GetDurationMilliSecs(_TStart, _TStop) ((_TStop.time*1000+_TStop.millitm) - (_TStart.time*1000+_TStart.millitm))
static int washere = 0;
void * func(void * arg)
{
PTW32_FTIME(&currSysTimeStart);
washere = 1;
assert(pthread_spin_lock(&lock) == 0);
assert(pthread_spin_unlock(&lock) == 0);
PTW32_FTIME(&currSysTimeStop);
return (void *)(size_t)GetDurationMilliSecs(currSysTimeStart, currSysTimeStop);
}
int
main()
{
void* result = (void*)0;
pthread_t t;
int CPUs;
PTW32_STRUCT_TIMEB sysTime;
if ((CPUs = pthread_num_processors_np()) == 1)
{
printf("Test not run - it requires multiple CPUs.\n");
exit(0);
}
assert(pthread_spin_lock(&lock) == 0);
assert(pthread_create(&t, NULL, func, NULL) == 0);
while (washere == 0)
{
sched_yield();
}
do
{
sched_yield();
PTW32_FTIME(&sysTime);
}
while (GetDurationMilliSecs(currSysTimeStart, sysTime) <= 1000);
assert(pthread_spin_unlock(&lock) == 0);
assert(pthread_join(t, &result) == 0);
assert((int)(size_t)result > 1000);
assert(pthread_spin_destroy(&lock) == 0);
assert(washere == 1);
return 0;
}
