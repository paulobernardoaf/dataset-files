#include "pthread.h"
#include "implement.h"
static struct pthread_mutexattr_t_ ptw32_recursive_mutexattr_s =
{PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_RECURSIVE};
static struct pthread_mutexattr_t_ ptw32_errorcheck_mutexattr_s =
{PTHREAD_PROCESS_PRIVATE, PTHREAD_MUTEX_ERRORCHECK};
static pthread_mutexattr_t ptw32_recursive_mutexattr = &ptw32_recursive_mutexattr_s;
static pthread_mutexattr_t ptw32_errorcheck_mutexattr = &ptw32_errorcheck_mutexattr_s;
INLINE int
ptw32_mutex_check_need_init (pthread_mutex_t * mutex)
{
register int result = 0;
register pthread_mutex_t mtx;
ptw32_mcs_local_node_t node;
ptw32_mcs_lock_acquire(&ptw32_mutex_test_init_lock, &node);
mtx = *mutex;
if (mtx == PTHREAD_MUTEX_INITIALIZER)
{
result = pthread_mutex_init (mutex, NULL);
}
else if (mtx == PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
{
result = pthread_mutex_init (mutex, &ptw32_recursive_mutexattr);
}
else if (mtx == PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
{
result = pthread_mutex_init (mutex, &ptw32_errorcheck_mutexattr);
}
else if (mtx == NULL)
{
result = EINVAL;
}
ptw32_mcs_lock_release(&node);
return (result);
}
