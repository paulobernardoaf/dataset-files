#include "pthread.h"
#include "implement.h"
int
pthread_equal (pthread_t t1, pthread_t t2)
{
int result;
result = ( t1.p == t2.p && t1.x == t2.x );
return (result);
} 
