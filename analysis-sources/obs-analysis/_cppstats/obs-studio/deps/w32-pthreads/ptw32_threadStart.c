#include "pthread.h"
#include "implement.h"
#include <stdio.h>
#if defined(__CLEANUP_C)
#include <setjmp.h>
#endif
#if defined(__CLEANUP_SEH)
static DWORD
ExceptionFilter (EXCEPTION_POINTERS * ep, DWORD * ei)
{
switch (ep->ExceptionRecord->ExceptionCode)
{
case EXCEPTION_PTW32_SERVICES:
{
DWORD param;
DWORD numParams = ep->ExceptionRecord->NumberParameters;
numParams = (numParams > 3) ? 3 : numParams;
for (param = 0; param < numParams; param++)
{
ei[param] = ep->ExceptionRecord->ExceptionInformation[param];
}
return EXCEPTION_EXECUTE_HANDLER;
break;
}
default:
{
pthread_t self = pthread_self ();
ptw32_callUserDestroyRoutines (self);
return EXCEPTION_CONTINUE_SEARCH;
break;
}
}
}
#elif defined(__CLEANUP_CXX)
#if defined(_MSC_VER)
#include <eh.h>
#elif defined(__WATCOMC__)
#include <eh.h>
#include <exceptio.h>
typedef terminate_handler
terminate_function;
#else
#if defined(__GNUC__) && __GNUC__ < 3
#include <new.h>
#else
#include <new>
using
std::terminate_handler;
using
std::terminate;
using
std::set_terminate;
#endif
typedef terminate_handler
terminate_function;
#endif
static terminate_function
ptw32_oldTerminate;
void
ptw32_terminate ()
{
set_terminate (ptw32_oldTerminate);
(void) pthread_win32_thread_detach_np ();
terminate ();
}
#endif
#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || (defined (__MSVCRT__) && ! defined (__DMC__))
unsigned
__stdcall
#else
void
#endif
ptw32_threadStart (void *vthreadParms)
{
ThreadParms * threadParms = (ThreadParms *) vthreadParms;
pthread_t self;
ptw32_thread_t * sp;
void * (PTW32_CDECL *start) (void *);
void * arg;
#if defined(__CLEANUP_SEH)
DWORD
ei[] = { 0, 0, 0 };
#endif
#if defined(__CLEANUP_C)
int setjmp_rc;
#endif
ptw32_mcs_local_node_t stateLock;
void * status = (void *) 0;
self = threadParms->tid;
sp = (ptw32_thread_t *) self.p;
start = threadParms->start;
arg = threadParms->arg;
free (threadParms);
#if (defined(__MINGW64__) || defined(__MINGW32__)) && ! defined (__MSVCRT__)
sp->thread = GetCurrentThreadId ();
ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
pthread_setspecific (ptw32_selfThreadKey, sp);
#else
pthread_setspecific (ptw32_selfThreadKey, sp);
ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
#endif
sp->state = PThreadStateRunning;
ptw32_mcs_lock_release (&stateLock);
#if defined(__CLEANUP_SEH)
__try
{
status = sp->exitStatus = (*start) (arg);
sp->state = PThreadStateExiting;
#if defined(_UWIN)
if (--pthread_count <= 0)
exit (0);
#endif
}
__except (ExceptionFilter (GetExceptionInformation (), ei))
{
switch (ei[0])
{
case PTW32_EPS_CANCEL:
status = sp->exitStatus = PTHREAD_CANCELED;
#if defined(_UWIN)
if (--pthread_count <= 0)
exit (0);
#endif
break;
case PTW32_EPS_EXIT:
status = sp->exitStatus;
break;
default:
status = sp->exitStatus = PTHREAD_CANCELED;
break;
}
}
#else 
#if defined(__CLEANUP_C)
setjmp_rc = setjmp (sp->start_mark);
if (0 == setjmp_rc)
{
status = sp->exitStatus = (*start) (arg);
sp->state = PThreadStateExiting;
}
else
{
switch (setjmp_rc)
{
case PTW32_EPS_CANCEL:
status = sp->exitStatus = PTHREAD_CANCELED;
break;
case PTW32_EPS_EXIT:
status = sp->exitStatus;
break;
default:
status = sp->exitStatus = PTHREAD_CANCELED;
break;
}
}
#else 
#if defined(__CLEANUP_CXX)
ptw32_oldTerminate = set_terminate (&ptw32_terminate);
try
{
try
{
status = sp->exitStatus = (*start) (arg);
sp->state = PThreadStateExiting;
}
catch (ptw32_exception &)
{
throw;
}
catch (...)
{
terminate_function
term_func = set_terminate (0);
set_terminate (term_func);
if (term_func != 0)
{
term_func ();
}
throw;
}
}
catch (ptw32_exception_cancel &)
{
status = sp->exitStatus = PTHREAD_CANCELED;
}
catch (ptw32_exception_exit &)
{
status = sp->exitStatus;
}
catch (...)
{
status = sp->exitStatus = PTHREAD_CANCELED;
}
(void) set_terminate (ptw32_oldTerminate);
#else
#error ERROR [__FILE__, line __LINE__]: Cleanup type undefined.
#endif 
#endif 
#endif 
#if defined(PTW32_STATIC_LIB)
(void) pthread_win32_thread_detach_np ();
#endif
#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)
_endthreadex ((unsigned)(size_t) status);
#else
_endthread ();
#endif
#if ! (defined(__MINGW64__) || defined(__MINGW32__)) || defined (__MSVCRT__) || defined (__DMC__)
return (unsigned)(size_t) status;
#endif
} 
