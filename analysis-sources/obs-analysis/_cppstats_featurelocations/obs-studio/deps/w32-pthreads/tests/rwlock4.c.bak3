













































#include "test.h"

pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;

static int washere = 0;

void * func(void * arg)
{
assert(pthread_rwlock_trywrlock(&rwlock1) == EBUSY);

washere = 1;

return 0; 
}

int
main()
{
pthread_t t;

assert(pthread_rwlock_rdlock(&rwlock1) == 0);

assert(pthread_create(&t, NULL, func, NULL) == 0);

assert(pthread_join(t, NULL) == 0);

assert(pthread_rwlock_unlock(&rwlock1) == 0);

assert(washere == 1);

return 0;
}
