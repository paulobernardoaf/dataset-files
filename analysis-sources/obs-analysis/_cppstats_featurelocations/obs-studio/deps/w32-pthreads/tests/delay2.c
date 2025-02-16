






































#include "test.h"

pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

void *
func(void * arg)
{
struct timespec interval = {5, 500000000L};

assert(pthread_mutex_lock(&mx) == 0);

#if defined(_MSC_VER)
#pragma inline_depth(0)
#endif
pthread_cleanup_push(pthread_mutex_unlock, &mx);
assert(pthread_delay_np(&interval) == 0);
pthread_cleanup_pop(1);
#if defined(_MSC_VER)
#pragma inline_depth()
#endif

return (void *)(size_t)1;
}

int
main(int argc, char * argv[])
{
pthread_t t;
void* result = (void*)0;

assert(pthread_mutex_lock(&mx) == 0);

assert(pthread_create(&t, NULL, func, NULL) == 0);
assert(pthread_cancel(t) == 0);

assert(pthread_mutex_unlock(&mx) == 0);

assert(pthread_join(t, &result) == 0);
assert(result == (void*)PTHREAD_CANCELED);

return 0;
}

