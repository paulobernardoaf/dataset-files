#include "test.h"
enum {
NUMTHREADS = 100
};
void *
func(void * arg)
{
int i = (int)(size_t)arg;
Sleep(i * 10);
pthread_exit(arg);
exit(1);
}
int
main(int argc, char * argv[])
{
pthread_t id[NUMTHREADS];
int i;
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_create(&id[i], NULL, func, (void *)(size_t)i) == 0);
}
Sleep(NUMTHREADS/2 * 10 + 50);
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_detach(id[i]) == 0);
}
Sleep(NUMTHREADS * 10 + 100);
for (i = 0; i < NUMTHREADS; i++)
{
assert(pthread_kill(id[i], 0) == ESRCH);
}
return 0;
}
