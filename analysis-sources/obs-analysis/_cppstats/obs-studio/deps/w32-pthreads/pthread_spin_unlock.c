#include "pthread.h"
#include "implement.h"
int
pthread_spin_unlock (pthread_spinlock_t * lock)
{
register pthread_spinlock_t s;
if (NULL == lock || NULL == *lock)
{
return (EINVAL);
}
s = *lock;
if (s == PTHREAD_SPINLOCK_INITIALIZER)
{
return EPERM;
}
switch ((long)
PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR) &s->interlock,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED))
{
case PTW32_SPIN_LOCKED:
case PTW32_SPIN_UNLOCKED:
return 0;
case PTW32_SPIN_USE_MUTEX:
return pthread_mutex_unlock (&(s->u.mutex));
}
return EINVAL;
}
