#include <dispatch/dispatch.h>
#include <errno.h>
#define sem_t dispatch_semaphore_t
#define sem_post(psem) dispatch_semaphore_signal(*psem)
#define sem_wait(psem) dispatch_semaphore_wait(*psem, DISPATCH_TIME_FOREVER)
#define sem_timedwait(psem, val) dispatch_semaphore_wait(*psem, dispatch_walltime(val, 0))
#define sem_destroy(psem) dispatch_release(*psem)
static inline int compat_sem_init(dispatch_semaphore_t *psem,
int unused, int val)
{
int ret = !!(*psem = dispatch_semaphore_create(val)) - 1;
if (ret < 0)
errno = ENOMEM;
return ret;
}
#define sem_init compat_sem_init
