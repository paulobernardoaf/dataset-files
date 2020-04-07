#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_thread.h"
#include <mmsystem.h>
#include <process.h>
static unsigned __stdcall thread_proc_trampoline(void *data)
{
_AL_THREAD *thread = data;
(*thread->proc)(thread, thread->arg);
_endthreadex(0);
return 0;
}
void _al_thread_create(_AL_THREAD *thread, void (*proc)(_AL_THREAD*, void*), void *arg)
{
ASSERT(thread);
ASSERT(proc);
{
InitializeCriticalSection(&thread->cs);
thread->should_stop = false;
thread->proc = proc;
thread->arg = arg;
thread->thread = (void *)_beginthreadex(NULL, 0,
thread_proc_trampoline, thread, 0, NULL);
}
}
void _al_thread_create_with_stacksize(_AL_THREAD* thread, void (*proc)(_AL_THREAD*, void*), void *arg, size_t stacksize) 
{
ASSERT(thread);
ASSERT(proc);
{
InitializeCriticalSection(&thread->cs);
thread->should_stop = false;
thread->proc = proc;
thread->arg = arg;
thread->thread = (void *)_beginthreadex(NULL, stacksize, 
thread_proc_trampoline, thread, 0, NULL);
}
}
void _al_thread_set_should_stop(_AL_THREAD *thread)
{
ASSERT(thread);
EnterCriticalSection(&thread->cs);
{
thread->should_stop = true;
}
LeaveCriticalSection(&thread->cs);
}
void _al_thread_join(_AL_THREAD *thread)
{
ASSERT(thread);
_al_thread_set_should_stop(thread);
WaitForSingleObject(thread->thread, INFINITE);
CloseHandle(thread->thread);
DeleteCriticalSection(&thread->cs);
}
void _al_thread_detach(_AL_THREAD *thread)
{
ASSERT(thread);
CloseHandle(thread->thread);
DeleteCriticalSection(&thread->cs);
}
void _al_mutex_init(_AL_MUTEX *mutex)
{
ASSERT(mutex);
if (!mutex->cs)
mutex->cs = al_malloc(sizeof *mutex->cs);
ASSERT(mutex->cs);
if (mutex->cs)
InitializeCriticalSection(mutex->cs);
else
abort();
}
void _al_mutex_init_recursive(_AL_MUTEX *mutex)
{
_al_mutex_init(mutex);
}
void _al_mutex_destroy(_AL_MUTEX *mutex)
{
ASSERT(mutex);
if (mutex->cs) {
DeleteCriticalSection(mutex->cs);
al_free(mutex->cs);
mutex->cs = NULL;
}
}
void _al_cond_init(_AL_COND *cond)
{
cond->nWaitersBlocked = 0;
cond->nWaitersGone = 0;
cond->nWaitersToUnblock = 0;
cond->semBlockQueue = CreateSemaphore(NULL, 0, INT_MAX, NULL);
InitializeCriticalSection(&cond->semBlockLock);
InitializeCriticalSection(&cond->mtxUnblockLock);
}
void _al_cond_destroy(_AL_COND *cond)
{
DeleteCriticalSection(&cond->mtxUnblockLock);
DeleteCriticalSection(&cond->semBlockLock);
CloseHandle(cond->semBlockQueue);
}
static int cond_wait(_AL_COND *cond, _AL_MUTEX *mtxExternal, DWORD timeout)
{
int nSignalsWasLeft;
bool bTimedOut;
DWORD dwWaitResult;
EnterCriticalSection(&cond->semBlockLock);
++cond->nWaitersBlocked;
LeaveCriticalSection(&cond->semBlockLock);
_al_mutex_unlock(mtxExternal);
dwWaitResult = WaitForSingleObject(cond->semBlockQueue, timeout);
if (dwWaitResult == WAIT_TIMEOUT)
bTimedOut = true;
else if (dwWaitResult == WAIT_OBJECT_0)
bTimedOut = false;
else {
_al_mutex_lock(mtxExternal);
ASSERT(false);
return 0;
}
EnterCriticalSection(&cond->mtxUnblockLock);
if (0 != (nSignalsWasLeft = cond->nWaitersToUnblock)) {
--(cond->nWaitersToUnblock);
}
else if (INT_MAX/2 == ++(cond->nWaitersGone)) { 
EnterCriticalSection(&cond->semBlockLock);
cond->nWaitersBlocked -= cond->nWaitersGone; 
LeaveCriticalSection(&cond->semBlockLock);
cond->nWaitersGone = 0;
}
LeaveCriticalSection(&cond->mtxUnblockLock);
if (1 == nSignalsWasLeft) {
LeaveCriticalSection(&cond->semBlockLock); 
}
_al_mutex_lock(mtxExternal);
return bTimedOut ? -1 : 0;
}
void _al_cond_wait(_AL_COND *cond, _AL_MUTEX *mtxExternal)
{
int result;
ASSERT(cond);
ASSERT(mtxExternal);
result = cond_wait(cond, mtxExternal, INFINITE);
ASSERT(result != -1);
(void)result;
}
int _al_cond_timedwait(_AL_COND *cond, _AL_MUTEX *mtxExternal,
const ALLEGRO_TIMEOUT *timeout)
{
ALLEGRO_TIMEOUT_WIN *win_timeout = (ALLEGRO_TIMEOUT_WIN *) timeout;
DWORD now;
DWORD rel_msecs;
ASSERT(cond);
ASSERT(mtxExternal);
now = timeGetTime();
rel_msecs = win_timeout->abstime - now;
if (rel_msecs > INT_MAX) {
rel_msecs = 0;
}
return cond_wait(cond, mtxExternal, rel_msecs);
}
static void cond_signal(_AL_COND *cond, bool bAll)
{
int nSignalsToIssue;
EnterCriticalSection(&cond->mtxUnblockLock);
if (0 != cond->nWaitersToUnblock) { 
if (0 == cond->nWaitersBlocked) { 
LeaveCriticalSection(&cond->mtxUnblockLock);
return;
}
if (bAll) {
cond->nWaitersToUnblock += (nSignalsToIssue = cond->nWaitersBlocked);
cond->nWaitersBlocked = 0;
}
else {
nSignalsToIssue = 1;
++cond->nWaitersToUnblock;
--cond->nWaitersBlocked;
}
}
else if (cond->nWaitersBlocked > cond->nWaitersGone) { 
EnterCriticalSection(&cond->semBlockLock); 
if (0 != cond->nWaitersGone) {
cond->nWaitersBlocked -= cond->nWaitersGone;
cond->nWaitersGone = 0;
}
if (bAll) {
nSignalsToIssue = (cond->nWaitersToUnblock = cond->nWaitersBlocked);
cond->nWaitersBlocked = 0;
}
else {
nSignalsToIssue = cond->nWaitersToUnblock = 1;
--cond->nWaitersBlocked;
}
}
else { 
LeaveCriticalSection(&cond->mtxUnblockLock);
return;
}
LeaveCriticalSection(&cond->mtxUnblockLock);
ReleaseSemaphore(cond->semBlockQueue, nSignalsToIssue, NULL);
return;
}
void _al_cond_broadcast(_AL_COND *cond)
{
cond_signal(cond, true);
}
void _al_cond_signal(_AL_COND *cond)
{
cond_signal(cond, false);
}
