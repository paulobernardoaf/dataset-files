#if ! defined(_UWIN) && ! defined(WINCE)
#include <process.h>
#endif
#if !defined(NEED_FTIME)
#include <sys/timeb.h>
#endif
#include "pthread.h"
#include "implement.h"
#include "ptw32_mutex_check_need_init.c"
#include "pthread_mutex_init.c"
#include "pthread_mutex_destroy.c"
#include "pthread_mutexattr_init.c"
#include "pthread_mutexattr_destroy.c"
#include "pthread_mutexattr_getpshared.c"
#include "pthread_mutexattr_setpshared.c"
#include "pthread_mutexattr_settype.c"
#include "pthread_mutexattr_gettype.c"
#include "pthread_mutexattr_setrobust.c"
#include "pthread_mutexattr_getrobust.c"
#include "pthread_mutex_lock.c"
#include "pthread_mutex_timedlock.c"
#include "pthread_mutex_unlock.c"
#include "pthread_mutex_trylock.c"
#include "pthread_mutex_consistent.c"
