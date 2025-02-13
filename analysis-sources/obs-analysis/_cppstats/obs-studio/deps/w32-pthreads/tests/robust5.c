#include "test.h"
static int lockCount;
static pthread_mutex_t mutex[3];
void * owner(void * arg)
{
assert(pthread_mutex_lock(&mutex[0]) == 0);
lockCount++;
assert(pthread_mutex_lock(&mutex[1]) == 0);
lockCount++;
assert(pthread_mutex_lock(&mutex[2]) == 0);
lockCount++;
return 0;
}
void * inheritor(void * arg)
{
assert(pthread_mutex_lock(&mutex[0]) == EOWNERDEAD);
lockCount++;
assert(pthread_mutex_lock(&mutex[1]) == EOWNERDEAD);
lockCount++;
assert(pthread_mutex_lock(&mutex[2]) == EOWNERDEAD);
lockCount++;
return 0;
}
int
main()
{
pthread_t to, ti;
pthread_mutexattr_t ma;
assert(pthread_mutexattr_init(&ma) == 0);
assert(pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST) == 0);
lockCount = 0;
assert(pthread_mutex_init(&mutex[0], &ma) == 0);
assert(pthread_mutex_init(&mutex[1], &ma) == 0);
assert(pthread_mutex_init(&mutex[2], &ma) == 0);
assert(pthread_create(&to, NULL, owner, NULL) == 0);
assert(pthread_join(to, NULL) == 0);
assert(pthread_create(&ti, NULL, inheritor, NULL) == 0);
assert(pthread_join(ti, NULL) == 0);
assert(lockCount == 6);
assert(pthread_mutex_lock(&mutex[0]) == EOWNERDEAD);
assert(pthread_mutex_consistent(&mutex[0]) == 0);
assert(pthread_mutex_unlock(&mutex[0]) == 0);
assert(pthread_mutex_destroy(&mutex[0]) == 0);
assert(pthread_mutex_lock(&mutex[1]) == EOWNERDEAD);
assert(pthread_mutex_consistent(&mutex[1]) == 0);
assert(pthread_mutex_unlock(&mutex[1]) == 0);
assert(pthread_mutex_destroy(&mutex[1]) == 0);
assert(pthread_mutex_lock(&mutex[2]) == EOWNERDEAD);
assert(pthread_mutex_consistent(&mutex[2]) == 0);
assert(pthread_mutex_unlock(&mutex[2]) == 0);
assert(pthread_mutex_destroy(&mutex[2]) == 0);
assert(pthread_mutexattr_destroy(&ma) == 0);
return 0;
}
