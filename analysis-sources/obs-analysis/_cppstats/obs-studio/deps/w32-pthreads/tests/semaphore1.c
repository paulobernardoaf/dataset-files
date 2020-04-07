#include "test.h"
void *
thr(void * arg)
{
sem_t s;
int result;
assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, 0) == 0);
assert((result = sem_trywait(&s)) == -1);
if ( result == -1 )
{
int err = errno;
if (err != EAGAIN)
{
printf("thread: sem_trywait 1: expecting error %s: got %s\n",
error_string[EAGAIN], error_string[err]); fflush(stdout);
}
assert(err == EAGAIN);
}
else
{
printf("thread: ok 1\n");
}
assert((result = sem_post(&s)) == 0);
assert((result = sem_trywait(&s)) == 0);
assert(sem_post(&s) == 0);
return NULL;
}
int
main()
{
pthread_t t;
sem_t s;
void* result1 = (void*)-1;
int result2;
assert(pthread_create(&t, NULL, thr, NULL) == 0);
assert(pthread_join(t, &result1) == 0);
assert((int)(size_t)result1 == 0);
assert(sem_init(&s, PTHREAD_PROCESS_PRIVATE, 0) == 0);
assert((result2 = sem_trywait(&s)) == -1);
if (result2 == -1)
{
int err = errno;
if (err != EAGAIN)
{
printf("main: sem_trywait 1: expecting error %s: got %s\n",
error_string[EAGAIN], error_string[err]); fflush(stdout);
}
assert(err == EAGAIN);
}
else
{
printf("main: ok 1\n");
}
assert((result2 = sem_post(&s)) == 0);
assert((result2 = sem_trywait(&s)) == 0);
assert(sem_post(&s) == 0);
return 0;
}
