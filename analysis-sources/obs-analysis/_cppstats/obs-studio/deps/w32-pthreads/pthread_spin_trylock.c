#include "pthread.h"
#include "implement.h"
int
pthread_spin_trylock (pthread_spinlock_t * lock)
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
switch ((long)
PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR) &s->interlock,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED))
{
case PTW32_SPIN_UNLOCKED:
return 0;
case PTW32_SPIN_LOCKED:
return EBUSY;
case PTW32_SPIN_USE_MUTEX:
return pthread_mutex_trylock (&(s->u.mutex));
}
return EINVAL;
}
