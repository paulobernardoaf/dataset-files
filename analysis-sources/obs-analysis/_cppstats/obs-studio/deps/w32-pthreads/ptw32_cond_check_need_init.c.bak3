




































#include "pthread.h"
#include "implement.h"


INLINE int
ptw32_cond_check_need_init (pthread_cond_t * cond)
{
int result = 0;
ptw32_mcs_local_node_t node;





ptw32_mcs_lock_acquire(&ptw32_cond_test_init_lock, &node);








if (*cond == PTHREAD_COND_INITIALIZER)
{
result = pthread_cond_init (cond, NULL);
}
else if (*cond == NULL)
{





result = EINVAL;
}

ptw32_mcs_lock_release(&node);

return result;
}
