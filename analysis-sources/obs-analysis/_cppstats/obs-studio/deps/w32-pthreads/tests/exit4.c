#include "test.h"
#if !defined(_UWIN)
#include <process.h>
#endif
enum {
NUMTHREADS = 4
};
typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
int count;
};
static bag_t threadbag[NUMTHREADS + 1];
#if ! defined (__MINGW32__) || defined (__MSVCRT__)
unsigned __stdcall
#else
void
#endif
Win32thread(void * arg)
{
int result = 1;
bag_t * bag = (bag_t *) arg;
assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;
pthread_exit((void *)(size_t)result);
return 0;
}
int
main()
{
int failed = 0;
int i;
HANDLE h[NUMTHREADS + 1];
unsigned thrAddr; 
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
#if ! defined (__MINGW32__) || defined (__MSVCRT__)
h[i] = (HANDLE) _beginthreadex(NULL, 0, Win32thread, (void *) &threadbag[i], 0, &thrAddr);
#else
h[i] = (HANDLE) _beginthread(Win32thread, 0, (void *) &threadbag[i]);
#endif
}
Sleep(500);
Sleep(NUMTHREADS * 100);
for (i = 1; i <= NUMTHREADS; i++)
{ 
if (!threadbag[i].started)
{
failed |= !threadbag[i].started;
fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
}
}
assert(!failed);
failed = 0;
for (i = 1; i <= NUMTHREADS; i++)
{
int fail = 0;
int result = 0;
#if ! defined (__MINGW32__) || defined (__MSVCRT__)
assert(GetExitCodeThread(h[i], (LPDWORD) &result) == TRUE);
#else
result = 1;
#endif
fail = (result != 1);
if (fail)
{
fprintf(stderr, "Thread %d: started %d: count %d\n",
i,
threadbag[i].started,
threadbag[i].count);
}
failed = (failed || fail);
}
assert(!failed);
return 0;
}
