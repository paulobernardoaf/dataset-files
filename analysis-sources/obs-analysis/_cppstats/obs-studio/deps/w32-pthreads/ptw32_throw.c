#include "pthread.h"
#include "implement.h"
#if defined(__CLEANUP_C)
#include <setjmp.h>
#endif
#if defined(_MSC_VER)
#pragma warning(disable:4290)
#endif
void
ptw32_throw (DWORD exception)
#if defined(__CLEANUP_CXX)
throw(ptw32_exception_cancel,ptw32_exception_exit)
#endif
{
ptw32_thread_t * sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);
#if defined(__CLEANUP_SEH)
DWORD exceptionInformation[3];
#endif
sp->state = PThreadStateExiting;
if (exception != PTW32_EPS_CANCEL && exception != PTW32_EPS_EXIT)
{
exit (1);
}
if (NULL == sp || sp->implicit)
{
#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)
unsigned exitCode = 0;
switch (exception)
{
case PTW32_EPS_CANCEL:
exitCode = (unsigned)(size_t) PTHREAD_CANCELED;
break;
case PTW32_EPS_EXIT:
if (NULL != sp)
{
exitCode = (unsigned)(size_t) sp->exitStatus;
}
break;
}
#endif
#if defined(PTW32_STATIC_LIB)
pthread_win32_thread_detach_np ();
#endif
#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)
_endthreadex (exitCode);
#else
_endthread ();
#endif
}
#if defined(__CLEANUP_SEH)
exceptionInformation[0] = (DWORD) (exception);
exceptionInformation[1] = (DWORD) (0);
exceptionInformation[2] = (DWORD) (0);
RaiseException (EXCEPTION_PTW32_SERVICES, 0, 3, exceptionInformation);
#else 
#if defined(__CLEANUP_C)
ptw32_pop_cleanup_all (1);
longjmp (sp->start_mark, exception);
#else 
#if defined(__CLEANUP_CXX)
switch (exception)
{
case PTW32_EPS_CANCEL:
throw ptw32_exception_cancel ();
break;
case PTW32_EPS_EXIT:
throw ptw32_exception_exit ();
break;
}
#else
#error ERROR [__FILE__, line __LINE__]: Cleanup type undefined.
#endif 
#endif 
#endif 
}
void
ptw32_pop_cleanup_all (int execute)
{
while (NULL != ptw32_pop_cleanup (execute))
{
}
}
DWORD
ptw32_get_exception_services_code (void)
{
#if defined(__CLEANUP_SEH)
return EXCEPTION_PTW32_SERVICES;
#else
return (DWORD)0;
#endif
}
