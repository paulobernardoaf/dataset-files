#include "test.h"
#include <sys/timeb.h>
#if defined(__GNUC__)
#include <stdlib.h>
#endif
#include "benchtest.h"
#define ITERATIONS 1000000L
sem_t sema;
HANDLE w32sema;
PTW32_STRUCT_TIMEB currSysTimeStart;
PTW32_STRUCT_TIMEB currSysTimeStop;
long durationMilliSecs;
long overHeadMilliSecs = 0;
int one = 1;
int zero = 0;
#define GetDurationMilliSecs(_TStart, _TStop) ((long)((_TStop.time*1000+_TStop.millitm) - (_TStart.time*1000+_TStart.millitm)))
#define TESTSTART { int i, j = 0, k = 0; PTW32_FTIME(&currSysTimeStart); for (i = 0; i < ITERATIONS; i++) { j++;
#define TESTSTOP }; PTW32_FTIME(&currSysTimeStop); if (j + k == i) j++; }
void
reportTest (char * testNameString)
{
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
printf( "%-45s %15ld %15.3f\n",
testNameString,
durationMilliSecs,
(float) durationMilliSecs * 1E3 / ITERATIONS);
}
int
main (int argc, char *argv[])
{
printf( "=============================================================================\n");
printf( "\nOperations on a semaphore.\n%ld iterations\n\n",
ITERATIONS);
printf( "%-45s %15s %15s\n",
"Test",
"Total(msec)",
"average(usec)");
printf( "-----------------------------------------------------------------------------\n");
TESTSTART
assert(1 == one);
TESTSTOP
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
overHeadMilliSecs = durationMilliSecs;
assert((w32sema = CreateSemaphore(NULL, (long) 0, (long) ITERATIONS, NULL)) != 0);
TESTSTART
assert((ReleaseSemaphore(w32sema, 1, NULL),1) == one);
TESTSTOP
assert(CloseHandle(w32sema) != 0);
reportTest("W32 Post with no waiters");
assert((w32sema = CreateSemaphore(NULL, (long) ITERATIONS, (long) ITERATIONS, NULL)) != 0);
TESTSTART
assert((WaitForSingleObject(w32sema, INFINITE),1) == one);
TESTSTOP
assert(CloseHandle(w32sema) != 0);
reportTest("W32 Wait without blocking");
assert(sem_init(&sema, 0, 0) == 0);
TESTSTART
assert((sem_post(&sema),1) == one);
TESTSTOP
assert(sem_destroy(&sema) == 0);
reportTest("POSIX Post with no waiters");
assert(sem_init(&sema, 0, ITERATIONS) == 0);
TESTSTART
assert((sem_wait(&sema),1) == one);
TESTSTOP
assert(sem_destroy(&sema) == 0);
reportTest("POSIX Wait without blocking");
printf( "=============================================================================\n");
return 0;
}
