





































#include "test.h"

void * func(void * arg)
{
Sleep(2000);
return 0;
}

int 
main()
{
pthread_t t1, t2;

assert(pthread_create(&t1, NULL, func, (void *) 1) == 0);

assert(pthread_create(&t2, NULL, func, (void *) 2) == 0);

assert(pthread_equal(t1, t2) == 0);

assert(pthread_equal(t1,t1) != 0);



Sleep(4000);


return 0;
}
