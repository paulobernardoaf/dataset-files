








































#include "test.h"

static int washere = 0;

void * func(void * arg)
{
washere = 1;
return 0; 
}

int
main()
{
pthread_t t;

assert(pthread_create(&t, NULL, func, NULL) == 0);



Sleep(2000);

assert(washere == 1);

return 0;
}
