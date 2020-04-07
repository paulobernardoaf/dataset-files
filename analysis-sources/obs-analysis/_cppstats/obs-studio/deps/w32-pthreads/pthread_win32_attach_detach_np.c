#include "pthread.h"
#include "implement.h"
#include <tchar.h>
static HINSTANCE ptw32_h_quserex;
BOOL
pthread_win32_process_attach_np ()
{
TCHAR QuserExDLLPathBuf[1024];
BOOL result = TRUE;
result = ptw32_processInitialize ();
#if defined(_UWIN)
pthread_count++;
#endif
#if defined(__GNUC__)
ptw32_features = 0;
#else
ptw32_features = PTW32_SYSTEM_INTERLOCKED_COMPARE_EXCHANGE;
#endif
#if defined(__GNUC__) || _MSC_VER < 1400
if(GetSystemDirectory(QuserExDLLPathBuf, sizeof(QuserExDLLPathBuf)))
{
(void) strncat(QuserExDLLPathBuf,
"\\QUSEREX.DLL",
sizeof(QuserExDLLPathBuf) - strlen(QuserExDLLPathBuf) - 1);
ptw32_h_quserex = LoadLibrary(QuserExDLLPathBuf);
}
#else
if(GetSystemDirectory(QuserExDLLPathBuf, sizeof(QuserExDLLPathBuf) / sizeof(TCHAR)) &&
0 == _tcsncat_s(QuserExDLLPathBuf, sizeof(QuserExDLLPathBuf) / sizeof(TCHAR), TEXT("\\QUSEREX.DLL"), 12))
{
ptw32_h_quserex = LoadLibrary(QuserExDLLPathBuf);
}
#endif
if (ptw32_h_quserex != NULL)
{
ptw32_register_cancelation = (DWORD (*)(PAPCFUNC, HANDLE, DWORD))
#if defined(NEED_UNICODE_CONSTS)
GetProcAddress (ptw32_h_quserex,
(const TCHAR *) TEXT ("QueueUserAPCEx"));
#else
GetProcAddress (ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx");
#endif
}
if (NULL == ptw32_register_cancelation)
{
ptw32_register_cancelation = ptw32_RegisterCancelation;
if (ptw32_h_quserex != NULL)
{
(void) FreeLibrary (ptw32_h_quserex);
}
ptw32_h_quserex = 0;
}
else
{
BOOL (*queue_user_apc_ex_init) (VOID);
queue_user_apc_ex_init = (BOOL (*)(VOID))
#if defined(NEED_UNICODE_CONSTS)
GetProcAddress (ptw32_h_quserex,
(const TCHAR *) TEXT ("QueueUserAPCEx_Init"));
#else
GetProcAddress (ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx_Init");
#endif
if (queue_user_apc_ex_init == NULL || !queue_user_apc_ex_init ())
{
ptw32_register_cancelation = ptw32_RegisterCancelation;
(void) FreeLibrary (ptw32_h_quserex);
ptw32_h_quserex = 0;
}
}
if (ptw32_h_quserex)
{
ptw32_features |= PTW32_ALERTABLE_ASYNC_CANCEL;
}
return result;
}
BOOL
pthread_win32_process_detach_np ()
{
if (ptw32_processInitialized)
{
ptw32_thread_t * sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);
if (sp != NULL)
{
if (sp->detachState == PTHREAD_CREATE_DETACHED)
{
ptw32_threadDestroy (sp->ptHandle);
TlsSetValue (ptw32_selfThreadKey->key, NULL);
}
}
ptw32_processTerminate ();
if (ptw32_h_quserex)
{
BOOL (*queue_user_apc_ex_fini) (VOID);
queue_user_apc_ex_fini = (BOOL (*)(VOID))
#if defined(NEED_UNICODE_CONSTS)
GetProcAddress (ptw32_h_quserex,
(const TCHAR *) TEXT ("QueueUserAPCEx_Fini"));
#else
GetProcAddress (ptw32_h_quserex, (LPCSTR) "QueueUserAPCEx_Fini");
#endif
if (queue_user_apc_ex_fini != NULL)
{
(void) queue_user_apc_ex_fini ();
}
(void) FreeLibrary (ptw32_h_quserex);
}
}
return TRUE;
}
BOOL
pthread_win32_thread_attach_np ()
{
return TRUE;
}
BOOL
pthread_win32_thread_detach_np ()
{
if (ptw32_processInitialized)
{
ptw32_thread_t * sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);
if (sp != NULL) 
{
ptw32_mcs_local_node_t stateLock;
ptw32_callUserDestroyRoutines (sp->ptHandle);
ptw32_mcs_lock_acquire (&sp->stateLock, &stateLock);
sp->state = PThreadStateLast;
ptw32_mcs_lock_release (&stateLock);
while (sp->robustMxList != NULL)
{
pthread_mutex_t mx = sp->robustMxList->mx;
ptw32_robust_mutex_remove(&mx, sp);
(void) PTW32_INTERLOCKED_EXCHANGE_LONG(
(PTW32_INTERLOCKED_LONGPTR)&mx->robustNode->stateInconsistent,
(PTW32_INTERLOCKED_LONG)-1);
SetEvent(mx->event);
}
if (sp->detachState == PTHREAD_CREATE_DETACHED)
{
ptw32_threadDestroy (sp->ptHandle);
TlsSetValue (ptw32_selfThreadKey->key, NULL);
}
}
}
return TRUE;
}
BOOL
pthread_win32_test_features_np (int feature_mask)
{
return ((ptw32_features & feature_mask) == feature_mask);
}
