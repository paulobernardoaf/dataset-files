#include "test.h"
static int lockCount;
static pthread_mutex_t mutex;
static pthread_mutexattr_t mxAttr;
void * locker(void * arg)
{
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
assert(pthread_mutex_trylock(&mutex) == EBUSY);
lockCount++;
assert(pthread_mutex_unlock(&mutex) == 0);
return (void *) 555;
}
int
main()
{
pthread_t t;
void* result = (void*)0;
int mxType = -1;
assert(pthread_mutexattr_init(&mxAttr) == 0);
BEGIN_MUTEX_STALLED_ROBUST(mxAttr)
lockCount = 0;
assert(pthread_mutexattr_settype(&mxAttr, PTHREAD_MUTEX_ERRORCHECK) == 0);
assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
assert(mxType == PTHREAD_MUTEX_ERRORCHECK);
assert(pthread_mutex_init(&mutex, &mxAttr) == 0);
assert(pthread_create(&t, NULL, locker, NULL) == 0);
assert(pthread_join(t, &result) == 0);
assert((int)(size_t)result == 555);
assert(lockCount == 2);
assert(pthread_mutex_destroy(&mutex) == 0);
END_MUTEX_STALLED_ROBUST(mxAttr)
assert(pthread_mutexattr_destroy(&mxAttr) == 0);
exit(0);
return 0;
}
