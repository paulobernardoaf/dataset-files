#include "test.h"
static int lockCount = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void * locker(void * arg)
{
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
assert(pthread_mutex_unlock(&mutex) == 0);
return 0;
}
int
main()
{
pthread_t t;
assert(mutex == PTHREAD_MUTEX_INITIALIZER);
assert(pthread_create(&t, NULL, locker, NULL) == 0);
Sleep(1000);
assert(lockCount == 1);
assert(pthread_mutex_unlock(&mutex) == 0);
Sleep (1000);
assert(lockCount == 2);
exit(0);
return 0;
}
