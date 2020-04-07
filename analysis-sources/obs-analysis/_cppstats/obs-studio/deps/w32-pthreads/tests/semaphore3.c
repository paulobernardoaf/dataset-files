#include "test.h"
#define MAX_COUNT 100
sem_t s;
void *
thr (void * arg)
{
assert(sem_wait(&s) == 0);
assert(pthread_detach(pthread_self()) == 0);
return NULL;
}
int
main()
{
int value = 0;
int i;
pthread_t t[MAX_COUNT+1];
assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, 0) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(value == 0);
for (i = 1; i <= MAX_COUNT; i++)
{
assert(pthread_create(&t[i], NULL, thr, NULL) == 0);
do {
sched_yield();
assert(sem_getvalue(&s, &value) == 0);
} while (value != -i);
assert(-value == i);
}
for (i = MAX_COUNT - 1; i >= 0; i--)
{
assert(sem_post(&s) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(-value == i);
}
return 0;
}
