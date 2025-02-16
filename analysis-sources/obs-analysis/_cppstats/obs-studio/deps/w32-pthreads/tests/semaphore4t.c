#include "test.h"
#define MAX_COUNT 100
sem_t s;
void *
thr (void * arg)
{
assert(sem_timedwait(&s, NULL) == 0);
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
assert(sem_getvalue(&s, &value) == 0);
assert(-value == MAX_COUNT);
assert(pthread_cancel(t[50]) == 0);
assert(pthread_join(t[50], NULL) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(-value == MAX_COUNT - 1);
for (i = MAX_COUNT - 2; i >= 0; i--)
{
assert(sem_post(&s) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(-value == i);
}
for (i = 1; i <= MAX_COUNT; i++)
if (i != 50)
assert(pthread_join(t[i], NULL) == 0);
return 0;
}
