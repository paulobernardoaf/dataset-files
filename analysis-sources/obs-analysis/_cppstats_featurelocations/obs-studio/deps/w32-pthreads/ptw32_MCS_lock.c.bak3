


























































































#include "pthread.h"
#include "sched.h"
#include "implement.h"







INLINE void 
ptw32_mcs_flag_set (HANDLE * flag)
{
HANDLE e = (HANDLE)(PTW32_INTERLOCKED_SIZE)PTW32_INTERLOCKED_COMPARE_EXCHANGE_SIZE(
(PTW32_INTERLOCKED_SIZEPTR)flag,
(PTW32_INTERLOCKED_SIZE)-1,
(PTW32_INTERLOCKED_SIZE)0);
if ((HANDLE)0 != e)
{

SetEvent(e);
}
}







INLINE void 
ptw32_mcs_flag_wait (HANDLE * flag)
{
if ((PTW32_INTERLOCKED_LONG)0 ==
PTW32_INTERLOCKED_EXCHANGE_ADD_SIZE((PTW32_INTERLOCKED_SIZEPTR)flag,
(PTW32_INTERLOCKED_SIZE)0)) 
{


HANDLE e = CreateEvent(NULL, PTW32_FALSE, PTW32_FALSE, NULL);

if ((PTW32_INTERLOCKED_SIZE)0 == PTW32_INTERLOCKED_COMPARE_EXCHANGE_SIZE(
(PTW32_INTERLOCKED_SIZEPTR)flag,
(PTW32_INTERLOCKED_SIZE)e,
(PTW32_INTERLOCKED_SIZE)0))
{

WaitForSingleObject(e, INFINITE);
}

CloseHandle(e);
}
}









#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif 
void 
ptw32_mcs_lock_acquire (ptw32_mcs_lock_t * lock, ptw32_mcs_local_node_t * node)
{
ptw32_mcs_local_node_t *pred;

node->lock = lock;
node->nextFlag = 0;
node->readyFlag = 0;
node->next = 0; 


pred = (ptw32_mcs_local_node_t *)PTW32_INTERLOCKED_EXCHANGE_PTR((PTW32_INTERLOCKED_PVOID_PTR)lock,
(PTW32_INTERLOCKED_PVOID)node);

if (0 != pred)
{

pred->next = node;
ptw32_mcs_flag_set(&pred->nextFlag);
ptw32_mcs_flag_wait(&node->readyFlag);
}
}









#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif 
void 
ptw32_mcs_lock_release (ptw32_mcs_local_node_t * node)
{
ptw32_mcs_lock_t *lock = node->lock;
ptw32_mcs_local_node_t *next =
(ptw32_mcs_local_node_t *)
PTW32_INTERLOCKED_EXCHANGE_ADD_SIZE((PTW32_INTERLOCKED_SIZEPTR)&node->next, (PTW32_INTERLOCKED_SIZE)0); 

if (0 == next)
{


if (node == (ptw32_mcs_local_node_t *)
PTW32_INTERLOCKED_COMPARE_EXCHANGE_PTR((PTW32_INTERLOCKED_PVOID_PTR)lock,
(PTW32_INTERLOCKED_PVOID)0,
(PTW32_INTERLOCKED_PVOID)node))
{

return;
}


ptw32_mcs_flag_wait(&node->nextFlag);
next = (ptw32_mcs_local_node_t *)
PTW32_INTERLOCKED_EXCHANGE_ADD_SIZE((PTW32_INTERLOCKED_SIZEPTR)&node->next, (PTW32_INTERLOCKED_SIZE)0); 
}


ptw32_mcs_flag_set(&next->readyFlag);
}




#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif 
int 
ptw32_mcs_lock_try_acquire (ptw32_mcs_lock_t * lock, ptw32_mcs_local_node_t * node)
{
node->lock = lock;
node->nextFlag = 0;
node->readyFlag = 0;
node->next = 0; 

return ((PTW32_INTERLOCKED_PVOID)PTW32_INTERLOCKED_COMPARE_EXCHANGE_PTR((PTW32_INTERLOCKED_PVOID_PTR)lock,
(PTW32_INTERLOCKED_PVOID)node,
(PTW32_INTERLOCKED_PVOID)0)
== (PTW32_INTERLOCKED_PVOID)0) ? 0 : EBUSY;
}












#if defined(PTW32_BUILD_INLINED)
INLINE 
#endif 
void 
ptw32_mcs_node_transfer (ptw32_mcs_local_node_t * new_node, ptw32_mcs_local_node_t * old_node)
{
new_node->lock = old_node->lock;
new_node->nextFlag = 0; 
new_node->readyFlag = 0; 
new_node->next = 0;

if ((ptw32_mcs_local_node_t *)PTW32_INTERLOCKED_COMPARE_EXCHANGE_PTR((PTW32_INTERLOCKED_PVOID_PTR)new_node->lock,
(PTW32_INTERLOCKED_PVOID)new_node,
(PTW32_INTERLOCKED_PVOID)old_node)
!= old_node)
{



while (old_node->next == 0)
{
sched_yield();
}
new_node->next = old_node->next;
}
}
