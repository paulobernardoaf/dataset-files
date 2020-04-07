#include "test.h"
#if defined(__cplusplus)
#if defined(_MSC_VER)
#include <eh.h>
#else
#if defined(__GNUC__) && __GNUC__ < 3
#include <new.h>
#else
#include <new>
using std::set_terminate;
#endif
#endif
enum {
NUMTHREADS = 1
};
int caught = 0;
pthread_mutex_t caughtLock;
void
terminateFunction ()
{
assert(pthread_mutex_lock(&caughtLock) == 0);
caught++;
#if 0
{
FILE * fp = fopen("pthread.log", "a");
fprintf(fp, "Caught = %d\n", caught);
fclose(fp);
}
#endif
assert(pthread_mutex_unlock(&caughtLock) == 0);
exit(0);
}
void *
exceptionedThread(void * arg)
{
int dummy = 0x1;
(void) set_terminate(&terminateFunction);
throw dummy;
return (void *) 0;
}
int
main()
{
int i;
pthread_t mt;
pthread_t et[NUMTHREADS];
pthread_mutexattr_t ma;
assert((mt = pthread_self()).p != NULL);
printf("See the notes inside of exception3.c re term_funcs.\n");
assert(pthread_mutexattr_init(&ma) == 0);
assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK) == 0);
assert(pthread_mutex_init(&caughtLock, &ma) == 0);
assert(pthread_mutexattr_destroy(&ma) == 0);
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_create(&et[i], NULL, exceptionedThread, NULL) == 0);
}
Sleep(NUMTHREADS * 100);
assert(caught == NUMTHREADS);
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
