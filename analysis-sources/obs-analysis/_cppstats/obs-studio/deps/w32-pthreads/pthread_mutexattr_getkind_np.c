#include "pthread.h"
#include "implement.h"
int
pthread_mutexattr_getkind_np (pthread_mutexattr_t * attr, int *kind)
{
return pthread_mutexattr_gettype (attr, kind);
}
