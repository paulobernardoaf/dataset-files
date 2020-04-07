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
pthread_t t;
assert(pthread_create(&t, NULL, func, (void *) NULL) == 0);
Sleep(100);
return 0;
}
