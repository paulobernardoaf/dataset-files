#include "pthread.h"
#include "implement.h"
#include "sched.h"
int
pthread_setschedparam (pthread_t thread, int policy,
const struct sched_param *param)
{
int result;
result = pthread_kill (thread, 0);
if (0 != result)
{
return result;
}
if (policy < SCHED_MIN || policy > SCHED_MAX)
{
return EINVAL;
}
if (policy != SCHED_OTHER)
{
return ENOTSUP;
}
return (ptw32_setthreadpriority (thread, policy, param->sched_priority));
}
int
ptw32_setthreadpriority (pthread_t thread, int policy, int priority)
{
int prio;
ptw32_mcs_local_node_t threadLock;
int result = 0;
ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;
prio = priority;
if (prio < sched_get_priority_min (policy) ||
prio > sched_get_priority_max (policy))
{
return EINVAL;
}
#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
#else
if (THREAD_PRIORITY_IDLE < prio && THREAD_PRIORITY_LOWEST > prio)
{
prio = THREAD_PRIORITY_LOWEST;
}
else if (THREAD_PRIORITY_TIME_CRITICAL > prio
&& THREAD_PRIORITY_HIGHEST < prio)
{
prio = THREAD_PRIORITY_HIGHEST;
}
#endif
ptw32_mcs_lock_acquire (&tp->threadLock, &threadLock);
if (0 == SetThreadPriority (tp->threadH, prio))
{
result = EINVAL;
}
else
{
tp->sched_priority = priority;
}
ptw32_mcs_lock_release (&threadLock);
return result;
}
