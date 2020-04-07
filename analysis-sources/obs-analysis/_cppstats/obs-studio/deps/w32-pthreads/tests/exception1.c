#if defined(_MSC_VER) || defined(__cplusplus)
#include "test.h"
enum {
NUMTHREADS = 4
};
void *
exceptionedThread(void * arg)
{
int dummy = 0;
void* result = (void*)((int)(size_t)PTHREAD_CANCELED + 1);
assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
Sleep(100);
#if defined(_MSC_VER) && !defined(__cplusplus)
__try
{
int zero = (int) (size_t)arg; 
int one = 1;
if (dummy == one/zero)
Sleep(0);
}
__except (EXCEPTION_EXECUTE_HANDLER)
{
result = (void*)((int)(size_t)PTHREAD_CANCELED + 2);
}
#elif defined(__cplusplus)
try
{
throw dummy;
}
#if defined(PtW32CatchAll)
PtW32CatchAll
#else
catch (...)
#endif
{
result = (void*)((int)(size_t)PTHREAD_CANCELED + 2);
}
#endif
return (void *) (size_t)result;
}
void *
canceledThread(void * arg)
{
void* result = (void*)((int)(size_t)PTHREAD_CANCELED + 1);
int count;
assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
#if defined(_MSC_VER) && !defined(__cplusplus)
__try
{
for (count = 0; count < 100; count++)
Sleep(100);
}
__except (EXCEPTION_EXECUTE_HANDLER)
{
result = (void*)((int)(size_t)PTHREAD_CANCELED + 2);
}
#elif defined(__cplusplus)
try
{
for (count = 0; count < 100; count++)
Sleep(100);
}
#if defined(PtW32CatchAll)
PtW32CatchAll
#else
catch (...)
#endif
{
result = (void*)((int)(size_t)PTHREAD_CANCELED + 2);
}
#endif
return (void *) (size_t)result;
}
int
main()
{
int failed = 0;
int i;
pthread_t mt;
pthread_t et[NUMTHREADS];
pthread_t ct[NUMTHREADS];
assert((mt = pthread_self()).p != NULL);
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_create(&et[i], NULL, exceptionedThread, (void *) 0) == 0);
assert(pthread_create(&ct[i], NULL, canceledThread, NULL) == 0);
}
Sleep(1000);
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_cancel(ct[i]) == 0);
}
Sleep(NUMTHREADS * 1000);
failed = 0;
for (i = 0; i < NUMTHREADS; i++)
{
int fail = 0;
void* result = (void*)0;
assert(pthread_join(ct[i], &result) == 0);
assert(!(fail = (result != PTHREAD_CANCELED)));
failed = (failed || fail);
assert(pthread_join(et[i], &result) == 0);
assert(!(fail = (result != (void*)((int)(size_t)PTHREAD_CANCELED + 2))));
failed = (failed || fail);
}
assert(!failed);
return 0;
}
#else 
#include <stdio.h>
int
main()
{
fprintf(stderr, "Test N/A for this compiler environment.\n");
return 0;
}
#endif 
