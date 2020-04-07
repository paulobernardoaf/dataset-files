#include "test.h"
static int lockCount = 0;
static pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
void * locker(void * arg)
{
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
assert(pthread_mutex_unlock(&mutex) == 0);
assert(pthread_mutex_unlock(&mutex) == 0);
return (void *) 555;
}
int
main()
{
pthread_t t;
void* result = (void*)0;
assert(mutex == PTHREAD_RECURSIVE_MUTEX_INITIALIZER);
assert(pthread_create(&t, NULL, locker, NULL) == 0);
assert(pthread_join(t, &result) == 0);
assert((int)(size_t)result == 555);
assert(lockCount == 2);
assert(pthread_mutex_destroy(&mutex) == 0);
exit(0);
return 0;
}
