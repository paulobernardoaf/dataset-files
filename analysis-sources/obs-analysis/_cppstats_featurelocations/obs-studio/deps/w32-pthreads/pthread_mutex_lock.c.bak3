



































#if !defined(_UWIN)

#endif
#include "pthread.h"
#include "implement.h"

int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
int kind;
pthread_mutex_t mx;
int result = 0;




if (*mutex == NULL)
{
return EINVAL;
}







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

if (PTHREAD_MUTEX_NORMAL == kind)
{
if ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1) != 0)
{
while ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
{
result = EINVAL;
break;
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
if (kind == PTHREAD_MUTEX_RECURSIVE)
{
mx->recursive_count++;
}
else
{
result = EDEADLK;
}
}
else
{
while ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
{
result = EINVAL;
break;
}
}

if (0 == result)
{
mx->recursive_count = 1;
mx->ownerThread = self;
}
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
if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
{
result = EINVAL;
break;
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
}
if (0 == result || EOWNERDEAD == result)
{




ptw32_robust_mutex_add(mutex, self);
}
}
else
{
if ((PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) 1,
(PTW32_INTERLOCKED_LONG) 0) == 0)
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
result = EDEADLK;
}
}
else
{
while (0 == (result = ptw32_robust_mutex_inherit(mutex))
&& (PTW32_INTERLOCKED_LONG) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR) &mx->lock_idx,
(PTW32_INTERLOCKED_LONG) -1) != 0)
{
if (WAIT_OBJECT_0 != WaitForSingleObject (mx->event, INFINITE))
{
result = EINVAL;
break;
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
mx->recursive_count = 1;




ptw32_robust_mutex_add(mutex, self);
}
}
}
}
}
}

return (result);
}

