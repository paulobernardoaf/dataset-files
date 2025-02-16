



































#include "pthread.h"
#include "implement.h"


int
pthread_spin_lock (pthread_spinlock_t * lock)
{
register pthread_spinlock_t s;

if (NULL == lock || NULL == *lock)
{
return (EINVAL);
}

if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
{
int result;

if ((result = ptw32_spinlock_check_need_init (lock)) != 0)
{
return (result);
}
}

s = *lock;

while ((PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED ==
PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR) &s->interlock,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED))
{
}

if (s->interlock == PTW32_SPIN_LOCKED)
{
return 0;
}
else if (s->interlock == PTW32_SPIN_USE_MUTEX)
{
return pthread_mutex_lock (&(s->u.mutex));
}

return EINVAL;
}
