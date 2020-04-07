












































#include "test.h"

pthread_mutex_t mutex = NULL;

int
main()
{
assert(mutex == NULL);

assert(pthread_mutex_init(&mutex, NULL) == 0);

assert(mutex != NULL);

assert(pthread_mutex_lock(&mutex) == 0);

assert(pthread_mutex_unlock(&mutex) == 0);

assert(pthread_mutex_destroy(&mutex) == 0);

assert(mutex == NULL);

return 0;
}
