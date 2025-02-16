#include "test.h"
void *
thr(void * arg)
{
assert(sem_post((sem_t *)arg) == 0);
return 0;
}
int
main()
{
pthread_t t;
sem_t s;
assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, 0) == 0);
assert(pthread_create(&t, NULL, thr, (void *)&s) == 0);
assert(sem_wait(&s) == 0);
assert(sem_destroy(&s) == 0);
assert(pthread_join(t, NULL) == 0);
return 0;
}
