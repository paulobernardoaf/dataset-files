#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
#if defined(_MSC_VER)
#pragma warning( disable : 4100 )
#endif
int
sem_open (const char *name, int oflag, mode_t mode, unsigned int value)
{
errno = ENOSYS;
return -1;
} 
