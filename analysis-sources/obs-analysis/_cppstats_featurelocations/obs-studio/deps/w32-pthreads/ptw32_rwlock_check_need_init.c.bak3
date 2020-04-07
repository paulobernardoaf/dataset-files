



































#include "pthread.h"
#include "implement.h"

INLINE int
ptw32_rwlock_check_need_init (pthread_rwlock_t * rwlock)
{
int result = 0;
ptw32_mcs_local_node_t node;





ptw32_mcs_lock_acquire(&ptw32_rwlock_test_init_lock, &node);









if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
{
result = pthread_rwlock_init (rwlock, NULL);
}
else if (*rwlock == NULL)
{





result = EINVAL;
}

ptw32_mcs_lock_release(&node);

return result;
}
