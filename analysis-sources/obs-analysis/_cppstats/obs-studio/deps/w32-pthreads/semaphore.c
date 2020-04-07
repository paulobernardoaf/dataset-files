#if !defined(NEED_FTIME)
#include <sys/timeb.h>
#endif
#include <limits.h>
#include "pthread.h"
#include "semaphore.h"
#include "implement.h"
#include "sem_init.c"
#include "sem_destroy.c"
#include "sem_trywait.c"
#include "sem_wait.c"
#include "sem_timedwait.c"
#include "sem_post.c"
#include "sem_post_multiple.c"
#include "sem_getvalue.c"
#include "sem_open.c"
#include "sem_close.c"
#include "sem_unlink.c"
