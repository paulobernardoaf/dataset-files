#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif
int
sem_unlink (const char *name)
{
errno = ENOSYS;
return -1;
} 
