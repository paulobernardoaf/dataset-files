


































#if !defined(_UWIN)

#endif
#include "pthread.h"
#include "implement.h"

INLINE
int
ptw32_robust_mutex_inherit(pthread_mutex_t * mutex)
{
int result;
pthread_mutex_t mx = *mutex;
ptw32_robust_node_t* robust = mx->robustNode;

switch ((LONG)PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR)&robust->stateInconsistent,
(PTW32_INTERLOCKED_LONG)PTW32_ROBUST_INCONSISTENT,
(PTW32_INTERLOCKED_LONG)-1 ))
{
case -1L:
result = EOWNERDEAD;
break;
case (LONG)PTW32_ROBUST_NOTRECOVERABLE:
result = ENOTRECOVERABLE;
break;
default:
result = 0;
break;
}

return result;
}













INLINE
void
ptw32_robust_mutex_add(pthread_mutex_t* mutex, pthread_t self)
{
ptw32_robust_node_t** list;
pthread_mutex_t mx = *mutex;
ptw32_thread_t* tp = (ptw32_thread_t*)self.p;
ptw32_robust_node_t* robust = mx->robustNode;

list = &tp->robustMxList;
mx->ownerThread = self;
if (NULL == *list)
{
robust->prev = NULL;
robust->next = NULL;
*list = robust;
}
else
{
robust->prev = NULL;
robust->next = *list;
(*list)->prev = robust;
*list = robust;
}
}

INLINE
void
ptw32_robust_mutex_remove(pthread_mutex_t* mutex, ptw32_thread_t* otp)
{
ptw32_robust_node_t** list;
pthread_mutex_t mx = *mutex;
ptw32_robust_node_t* robust = mx->robustNode;

list = &(((ptw32_thread_t*)mx->ownerThread.p)->robustMxList);
mx->ownerThread.p = otp;
if (robust->next != NULL)
{
robust->next->prev = robust->prev;
}
if (robust->prev != NULL)
{
robust->prev->next = robust->next;
}
if (*list == robust)
{
*list = robust->next;
}
}


int
pthread_mutex_consistent (pthread_mutex_t* mutex)
{
pthread_mutex_t mx = *mutex;
int result = 0;




if (mx == NULL)
{
return EINVAL;
}

if (mx->kind >= 0
|| (PTW32_INTERLOCKED_LONG)PTW32_ROBUST_INCONSISTENT != PTW32_INTERLOCKED_COMPARE_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR)&mx->robustNode->stateInconsistent,
(PTW32_INTERLOCKED_LONG)PTW32_ROBUST_CONSISTENT,
(PTW32_INTERLOCKED_LONG)PTW32_ROBUST_INCONSISTENT))
{
result = EINVAL;
}

return (result);
}

