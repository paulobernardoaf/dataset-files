#include "test.h"
pthread_mutex_t mutex = NULL;
pthread_mutexattr_t mxAttr;
int
main()
{
assert(pthread_mutexattr_init(&mxAttr) == 0);
BEGIN_MUTEX_STALLED_ROBUST(mxAttr)
assert(pthread_mutexattr_settype(&mxAttr, PTHREAD_MUTEX_RECURSIVE) == 0);
assert(mutex == NULL);
assert(pthread_mutex_init(&mutex, &mxAttr) == 0);
assert(mutex != NULL);
assert(pthread_mutex_lock(&mutex) == 0);
assert(pthread_mutex_unlock(&mutex) == 0);
assert(pthread_mutex_destroy(&mutex) == 0);
assert(mutex == NULL);
END_MUTEX_STALLED_ROBUST(mxAttr)
return 0;
}
