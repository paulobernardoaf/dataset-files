#include "pthread.h"
#include "implement.h"
static INLINE int
ptw32_timed_eventwait (HANDLE event, const struct timespec *abstime)
{
DWORD milliseconds;
DWORD status;
if (event == NULL)
{
return EINVAL;
}
else
{
if (abstime == NULL)
{
milliseconds = INFINITE;
}
else
{
milliseconds = ptw32_relmillisecs (abstime);
}
status = WaitForSingleObject (event, milliseconds);
if (status == WAIT_OBJECT_0)
{
return 0;
}
else if (status == WAIT_TIMEOUT)
{
return ETIMEDOUT;
}
else
{
return EINVAL;
}
}
return 0;
} 
int
pthread_mutex_timedlock (pthread_mutex_t * mutex,
const struct timespec *abstime)
{
pthread_mutex_t mx;
int kind;
int result = 0;
if (*mutex >= PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
{
if ((result = ptw32_mutex_check_need_init (mutex)) != 0)
{
return (result);
}
}
mx = *mutex;
kind = mx->kind;
if (kind >= 0)
{
if (mx->kind == PTHREAD_MUTEX_NORMAL)
{
if ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1) != 0)
{
while ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (0 != (result = ptw32_timed_eventwait (mx->event, abstime)))
{
return result;
}
}
}
}
else
{
pthread_t self = pthread_self();
if ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1,
(PTW32_INTERLOCKED_LONG) 0) == 0)
{
mx->recursive_count = 1;
mx->ownerThread = self;
}
else
{
if (pthread_equal (mx->ownerThread, self))
{
if (mx->kind == PTHREAD_MUTEX_RECURSIVE)
{
mx->recursive_count++;
}
else
{
return EDEADLK;
}
}
else
{
while ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (0 != (result = ptw32_timed_eventwait (mx->event, abstime)))
{
return result;
}
}
mx->recursive_count = 1;
mx->ownerThread = self;
}
}
}
}
else
{
ptw32_robust_state_t* statePtr = &mx->robustNode->stateInconsistent;
if ((PTW32_INTERLOCKED_LONG)PTW32_ROBUST_NOTRECOVERABLE == PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
(PTW32_INTERLOCKED_LONGPTR)statePtr,
(PTW32_INTERLOCKED_LONG)0))
{
result = ENOTRECOVERABLE;
}
else
{
pthread_t self = pthread_self();
kind = -kind - 1; 
if (PTHREAD_MUTEX_NORMAL == kind)
{
if ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1) != 0)
{
while (0 == (result = ptw32_robust_mutex_inherit(mutex))
&& (PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (0 != (result = ptw32_timed_eventwait (mx->event, abstime)))
{
return result;
}
if ((PTW32_INTERLOCKED_LONG)PTW32_ROBUST_NOTRECOVERABLE ==
PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
(PTW32_INTERLOCKED_LONGPTR)statePtr,
(PTW32_INTERLOCKED_LONG)0))
{
SetEvent(mx->event);
result = ENOTRECOVERABLE;
break;
}
}
if (0 == result || EOWNERDEAD == result)
{
ptw32_robust_mutex_add(mutex, self);
}
}
}
else
{
pthread_t self = pthread_self();
if (0 == (PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1,
(PTW32_INTERLOCKED_LONG) 0))
{
mx->recursive_count = 1;
ptw32_robust_mutex_add(mutex, self);
}
else
{
if (pthread_equal (mx->ownerThread, self))
{
if (PTHREAD_MUTEX_RECURSIVE == kind)
{
mx->recursive_count++;
}
else
{
return EDEADLK;
}
}
else
{
while (0 == (result = ptw32_robust_mutex_inherit(mutex))
&& (PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (0 != (result = ptw32_timed_eventwait (mx->event, abstime)))
{
return result;
}
}
if ((PTW32_INTERLOCKED_LONG)PTW32_ROBUST_NOTRECOVERABLE ==
PTW32_INTERLOCKED_EXCHANGE_ADD_LONG(
(PTW32_INTERLOCKED_LONGPTR)statePtr,
(PTW32_INTERLOCKED_LONG)0))
{
SetEvent(mx->event);
result = ENOTRECOVERABLE;
}
else if (0 == result || EOWNERDEAD == result)
{
mx->recursive_count = 1;
ptw32_robust_mutex_add(mutex, self);
}
}
}
}
}
}
return result;
}
