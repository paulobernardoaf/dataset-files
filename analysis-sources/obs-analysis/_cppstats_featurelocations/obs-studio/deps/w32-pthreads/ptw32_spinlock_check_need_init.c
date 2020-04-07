



































#include "pthread.h"
#include "implement.h"


INLINE int
ptw32_spinlock_check_need_init (pthread_spinlock_t * lock)
{
int result = 0;
ptw32_mcs_local_node_t node;





ptw32_mcs_lock_acquire(&ptw32_spinlock_test_init_lock, &node);









if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
{
result = pthread_spin_init (lock, PTHREAD_PROCESS_PRIVATE);
}
else if (*lock == NULL)
{





result = EINVAL;
}

ptw32_mcs_lock_release(&node);

return (result);
}
