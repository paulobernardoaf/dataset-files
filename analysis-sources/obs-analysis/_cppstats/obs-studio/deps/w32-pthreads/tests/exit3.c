#include "test.h"
void *
func(void * arg)
{
pthread_exit(arg);
assert(0);
return NULL;
}
int
main(int argc, char * argv[])
{
pthread_t id[4];
int i;
for (i = 0; i < 4; i++)
{
assert(pthread_create(&id[i], NULL, func, (void *)(size_t)i) == 0);
}
Sleep(400);
return 0;
}
