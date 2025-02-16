#include "pthread.h"
#include "implement.h"
#if !defined(_UWIN)
#endif
void
pthread_exit (void *value_ptr)
{
ptw32_thread_t * sp;
sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);
#if defined(_UWIN)
if (--pthread_count <= 0)
exit ((int) value_ptr);
#endif
if (NULL == sp)
{
#if ! (defined (__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)
_endthreadex ((unsigned) (size_t) value_ptr);
#else
_endthread ();
#endif
}
sp->exitStatus = value_ptr;
ptw32_throw (PTW32_EPS_EXIT);
}
