



































#include "pthread.h"
#include "implement.h"

























void *
pthread_timechange_handler_np (void *arg)
























{
int result = 0;
pthread_cond_t cv;
ptw32_mcs_local_node_t node;

ptw32_mcs_lock_acquire(&ptw32_cond_list_lock, &node);

cv = ptw32_cond_list_head;

while (cv != NULL && 0 == result)
{
result = pthread_cond_broadcast (&cv);
cv = cv->next;
}

ptw32_mcs_lock_release(&node);

return (void *) (size_t) (result != 0 ? EAGAIN : 0);
}
