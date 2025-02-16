



































#include "pthread.h"
#include "implement.h"


int
pthread_spin_destroy (pthread_spinlock_t * lock)
{
register pthread_spinlock_t s;
int result = 0;

if (lock == NULL || *lock == NULL)
{
return EINVAL;
}

if ((s = *lock) != PTHREAD_SPINLOCK_INITIALIZER)
{
if (s->interlock == PTW32_SPIN_USE_MUTEX)
{
result = pthread_mutex_destroy (&(s->u.mutex));
}
else if ((PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED !=
PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG ((PTW32_INTERLOCKED_LONGPTR) &s->interlock,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_INVALID,
(PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED))
{
result = EINVAL;
}

if (0 == result)
{




*lock = NULL;
(void) free (s);
}
}
else
{



ptw32_mcs_local_node_t node;

ptw32_mcs_lock_acquire(&ptw32_spinlock_test_init_lock, &node);




if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
{






*lock = NULL;
}
else
{




result = EBUSY;
}

ptw32_mcs_lock_release(&node);
}

return (result);
}
