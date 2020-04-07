#include "test.h"
static int lockCount;
static pthread_mutex_t mutex;
void * owner(void * arg)
{
assert(pthread_mutex_lock(&mutex) == 0);
lockCount++;
Sleep(200);
return 0;
}
void * inheritor(void * arg)
{
assert(pthread_mutex_lock(&mutex) == EOWNERDEAD);
lockCount++;
assert(pthread_mutex_unlock(&mutex) == 0);
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
assert(pthread_mutex_init(&mutex, &ma) == 0);
assert(pthread_create(&to, NULL, owner, NULL) == 0);
Sleep(100);
assert(pthread_create(&ti, NULL, inheritor, NULL) == 0);
assert(pthread_join(to, NULL) == 0);
assert(pthread_join(ti, NULL) == 0);
assert(lockCount == 2);
assert(pthread_mutex_lock(&mutex) == ENOTRECOVERABLE);
assert(pthread_mutex_destroy(&mutex) == 0);
lockCount = 0;
assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_NORMAL) == 0);
assert(pthread_mutex_init(&mutex, &ma) == 0);
assert(pthread_create(&to, NULL, owner, NULL) == 0);
Sleep(100);
assert(pthread_create(&ti, NULL, inheritor, NULL) == 0);
assert(pthread_join(to, NULL) == 0);
assert(pthread_join(ti, NULL) == 0);
assert(lockCount == 2);
assert(pthread_mutex_lock(&mutex) == ENOTRECOVERABLE);
assert(pthread_mutex_destroy(&mutex) == 0);
lockCount = 0;
assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK) == 0);
assert(pthread_mutex_init(&mutex, &ma) == 0);
assert(pthread_create(&to, NULL, owner, NULL) == 0);
Sleep(100);
assert(pthread_create(&ti, NULL, inheritor, NULL) == 0);
assert(pthread_join(to, NULL) == 0);
assert(pthread_join(ti, NULL) == 0);
assert(lockCount == 2);
assert(pthread_mutex_lock(&mutex) == ENOTRECOVERABLE);
assert(pthread_mutex_destroy(&mutex) == 0);
lockCount = 0;
assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE) == 0);
assert(pthread_mutex_init(&mutex, &ma) == 0);
assert(pthread_create(&to, NULL, owner, NULL) == 0);
Sleep(100);
assert(pthread_create(&ti, NULL, inheritor, NULL) == 0);
assert(pthread_join(to, NULL) == 0);
assert(pthread_join(ti, NULL) == 0);
assert(lockCount == 2);
assert(pthread_mutex_lock(&mutex) == ENOTRECOVERABLE);
assert(pthread_mutex_destroy(&mutex) == 0);
assert(pthread_mutexattr_destroy(&ma) == 0);
return 0;
}
