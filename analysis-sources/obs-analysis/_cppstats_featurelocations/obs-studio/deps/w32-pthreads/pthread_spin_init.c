



































#include "pthread.h"
#include "implement.h"


int
pthread_spin_init (pthread_spinlock_t * lock, int pshared)
{
pthread_spinlock_t s;
int cpus = 0;
int result = 0;

if (lock == NULL)
{
return EINVAL;
}

if (0 != ptw32_getprocessors (&cpus))
{
cpus = 1;
}

if (cpus > 1)
{
if (pshared == PTHREAD_PROCESS_SHARED)
{




#if _POSIX_THREAD_PROCESS_SHARED >= 0





#error ERROR [__FILE__, line __LINE__]: Process shared spin locks are not supported yet.

#else

return ENOSYS;

#endif 

}
}

s = (pthread_spinlock_t) calloc (1, sizeof (*s));

if (s == NULL)
{
return ENOMEM;
}

if (cpus > 1)
{
s->u.cpus = cpus;
s->interlock = PTW32_SPIN_UNLOCKED;
}
else
{
pthread_mutexattr_t ma;
result = pthread_mutexattr_init (&ma);

if (0 == result)
{
ma->pshared = pshared;
result = pthread_mutex_init (&(s->u.mutex), &ma);
if (0 == result)
{
s->interlock = PTW32_SPIN_USE_MUTEX;
}
}
(void) pthread_mutexattr_destroy (&ma);
}

if (0 == result)
{
*lock = s;
}
else
{
(void) free (s);
*lock = NULL;
}

return (result);
}
