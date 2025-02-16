#include "test.h"
#define MAX_COUNT 100
int
main()
{
sem_t s;
int value = 0;
int i;
assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, MAX_COUNT) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(value == MAX_COUNT);
for (i = MAX_COUNT - 1; i >= 0; i--)
{
assert(sem_wait(&s) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(value == i);
}
for (i = 1; i <= MAX_COUNT; i++)
{
assert(sem_post(&s) == 0);
assert(sem_getvalue(&s, &value) == 0);
assert(value == i);
}
return 0;
}
