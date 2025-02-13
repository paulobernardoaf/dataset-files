#include "sysdefs.h"
#if defined(HAVE_PTHREAD)
#include <pthread.h>
#define mythread_once(func) do { static pthread_once_t once_ = PTHREAD_ONCE_INIT; pthread_once(&once_, &func); } while (0)
#define mythread_sigmask(how, set, oset) pthread_sigmask(how, set, oset)
#else
#define mythread_once(func) do { static bool once_ = false; if (!once_) { func(); once_ = true; } } while (0)
#define mythread_sigmask(how, set, oset) sigprocmask(how, set, oset)
#endif
