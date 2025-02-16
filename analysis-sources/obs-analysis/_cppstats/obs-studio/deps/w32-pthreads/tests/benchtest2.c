#include "test.h"
#include <sys/timeb.h>
#if defined(__GNUC__)
#include <stdlib.h>
#endif
#include "benchtest.h"
#define PTW32_MUTEX_TYPES
#define ITERATIONS 100000L
pthread_mutex_t gate1, gate2;
old_mutex_t ox1, ox2;
CRITICAL_SECTION cs1, cs2;
pthread_mutexattr_t ma;
long durationMilliSecs;
long overHeadMilliSecs = 0;
PTW32_STRUCT_TIMEB currSysTimeStart;
PTW32_STRUCT_TIMEB currSysTimeStop;
pthread_t worker;
int running = 0;
#define GetDurationMilliSecs(_TStart, _TStop) ((long)((_TStop.time*1000+_TStop.millitm) - (_TStart.time*1000+_TStart.millitm)))
#define TESTSTART { int i, j = 0, k = 0; PTW32_FTIME(&currSysTimeStart); for (i = 0; i < ITERATIONS; i++) { j++;
#define TESTSTOP }; PTW32_FTIME(&currSysTimeStop); if (j + k == i) j++; }
void *
overheadThread(void * arg)
{
do
{
sched_yield();
}
while (running);
return NULL;
}
void *
oldThread(void * arg)
{
do
{
(void) old_mutex_lock(&ox1);
(void) old_mutex_lock(&ox2);
(void) old_mutex_unlock(&ox1);
sched_yield();
(void) old_mutex_unlock(&ox2);
}
while (running);
return NULL;
}
void *
workerThread(void * arg)
{
do
{
(void) pthread_mutex_lock(&gate1);
(void) pthread_mutex_lock(&gate2);
(void) pthread_mutex_unlock(&gate1);
sched_yield();
(void) pthread_mutex_unlock(&gate2);
}
while (running);
return NULL;
}
void *
CSThread(void * arg)
{
do
{
EnterCriticalSection(&cs1);
EnterCriticalSection(&cs2);
LeaveCriticalSection(&cs1);
sched_yield();
LeaveCriticalSection(&cs2);
}
while (running);
return NULL;
}
void
runTest (char * testNameString, int mType)
{
#if defined(PTW32_MUTEX_TYPES)
assert(pthread_mutexattr_settype(&ma, mType) == 0);
#endif
assert(pthread_mutex_init(&gate1, &ma) == 0);
assert(pthread_mutex_init(&gate2, &ma) == 0);
assert(pthread_mutex_lock(&gate1) == 0);
assert(pthread_mutex_lock(&gate2) == 0);
running = 1;
assert(pthread_create(&worker, NULL, workerThread, NULL) == 0);
TESTSTART
(void) pthread_mutex_unlock(&gate1);
sched_yield();
(void) pthread_mutex_unlock(&gate2);
(void) pthread_mutex_lock(&gate1);
(void) pthread_mutex_lock(&gate2);
TESTSTOP
running = 0;
assert(pthread_mutex_unlock(&gate2) == 0);
assert(pthread_mutex_unlock(&gate1) == 0);
assert(pthread_join(worker, NULL) == 0);
assert(pthread_mutex_destroy(&gate2) == 0);
assert(pthread_mutex_destroy(&gate1) == 0);
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
printf( "%-45s %15ld %15.3f\n",
testNameString,
durationMilliSecs,
(float) durationMilliSecs * 1E3 / ITERATIONS / 4 );
}
int
main (int argc, char *argv[])
{
assert(pthread_mutexattr_init(&ma) == 0);
printf( "=============================================================================\n");
printf( "\nLock plus unlock on a locked mutex.\n");
printf("%ld iterations, four locks/unlocks per iteration.\n\n", ITERATIONS);
printf( "%-45s %15s %15s\n",
"Test",
"Total(msec)",
"average(usec)");
printf( "-----------------------------------------------------------------------------\n");
running = 1;
assert(pthread_create(&worker, NULL, overheadThread, NULL) == 0);
TESTSTART
sched_yield();
sched_yield();
TESTSTOP
running = 0;
assert(pthread_join(worker, NULL) == 0);
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
overHeadMilliSecs = durationMilliSecs;
InitializeCriticalSection(&cs1);
InitializeCriticalSection(&cs2);
EnterCriticalSection(&cs1);
EnterCriticalSection(&cs2);
running = 1;
assert(pthread_create(&worker, NULL, CSThread, NULL) == 0);
TESTSTART
LeaveCriticalSection(&cs1);
sched_yield();
LeaveCriticalSection(&cs2);
EnterCriticalSection(&cs1);
EnterCriticalSection(&cs2);
TESTSTOP
running = 0;
LeaveCriticalSection(&cs2);
LeaveCriticalSection(&cs1);
assert(pthread_join(worker, NULL) == 0);
DeleteCriticalSection(&cs2);
DeleteCriticalSection(&cs1);
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
printf( "%-45s %15ld %15.3f\n",
"Simple Critical Section",
durationMilliSecs,
(float) durationMilliSecs * 1E3 / ITERATIONS / 4 );
old_mutex_use = OLD_WIN32CS;
assert(old_mutex_init(&ox1, NULL) == 0);
assert(old_mutex_init(&ox2, NULL) == 0);
assert(old_mutex_lock(&ox1) == 0);
assert(old_mutex_lock(&ox2) == 0);
running = 1;
assert(pthread_create(&worker, NULL, oldThread, NULL) == 0);
TESTSTART
(void) old_mutex_unlock(&ox1);
sched_yield();
(void) old_mutex_unlock(&ox2);
(void) old_mutex_lock(&ox1);
(void) old_mutex_lock(&ox2);
TESTSTOP
running = 0;
assert(old_mutex_unlock(&ox1) == 0);
assert(old_mutex_unlock(&ox2) == 0);
assert(pthread_join(worker, NULL) == 0);
assert(old_mutex_destroy(&ox2) == 0);
assert(old_mutex_destroy(&ox1) == 0);
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
printf( "%-45s %15ld %15.3f\n",
"Old PT Mutex using a Critical Section (WNT)",
durationMilliSecs,
(float) durationMilliSecs * 1E3 / ITERATIONS / 4);
old_mutex_use = OLD_WIN32MUTEX;
assert(old_mutex_init(&ox1, NULL) == 0);
assert(old_mutex_init(&ox2, NULL) == 0);
assert(old_mutex_lock(&ox1) == 0);
assert(old_mutex_lock(&ox2) == 0);
running = 1;
assert(pthread_create(&worker, NULL, oldThread, NULL) == 0);
TESTSTART
(void) old_mutex_unlock(&ox1);
sched_yield();
(void) old_mutex_unlock(&ox2);
(void) old_mutex_lock(&ox1);
(void) old_mutex_lock(&ox2);
TESTSTOP
running = 0;
assert(old_mutex_unlock(&ox1) == 0);
assert(old_mutex_unlock(&ox2) == 0);
assert(pthread_join(worker, NULL) == 0);
assert(old_mutex_destroy(&ox2) == 0);
assert(old_mutex_destroy(&ox1) == 0);
durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
printf( "%-45s %15ld %15.3f\n",
"Old PT Mutex using a Win32 Mutex (W9x)",
durationMilliSecs,
(float) durationMilliSecs * 1E3 / ITERATIONS / 4);
printf( ".............................................................................\n");
#if defined(PTW32_MUTEX_TYPES)
runTest("PTHREAD_MUTEX_DEFAULT", PTHREAD_MUTEX_DEFAULT);
runTest("PTHREAD_MUTEX_NORMAL", PTHREAD_MUTEX_NORMAL);
runTest("PTHREAD_MUTEX_ERRORCHECK", PTHREAD_MUTEX_ERRORCHECK);
runTest("PTHREAD_MUTEX_RECURSIVE", PTHREAD_MUTEX_RECURSIVE);
#else
runTest("Non-blocking lock", 0);
#endif
printf( ".............................................................................\n");
pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
#if defined(PTW32_MUTEX_TYPES)
runTest("PTHREAD_MUTEX_DEFAULT (Robust)", PTHREAD_MUTEX_DEFAULT);
runTest("PTHREAD_MUTEX_NORMAL (Robust)", PTHREAD_MUTEX_NORMAL);
runTest("PTHREAD_MUTEX_ERRORCHECK (Robust)", PTHREAD_MUTEX_ERRORCHECK);
runTest("PTHREAD_MUTEX_RECURSIVE (Robust)", PTHREAD_MUTEX_RECURSIVE);
#else
runTest("Non-blocking lock", 0);
#endif
printf( "=============================================================================\n");
pthread_mutexattr_destroy(&ma);
return 0;
}
