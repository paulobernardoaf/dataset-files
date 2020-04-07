#define _WIN32_WINNT 0x400
#include "test.h"
#include "../implement.h"
#include "../context.h"
static int washere = 0;
static void * func(void * arg)
{
washere = 1;
Sleep(1000);
return 0; 
}
static void
anotherEnding ()
{
washere++;
pthread_exit(0);
}
int
main()
{
pthread_t t;
HANDLE hThread;
assert(pthread_create(&t, NULL, func, NULL) == 0);
hThread = ((ptw32_thread_t *)t.p)->threadH;
Sleep(500);
SuspendThread(hThread);
if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT) 
{
CONTEXT context;
context.ContextFlags = CONTEXT_CONTROL;
GetThreadContext(hThread, &context);
PTW32_PROGCTR (context) = (DWORD_PTR) anotherEnding;
SetThreadContext(hThread, &context);
ResumeThread(hThread);
}
else
{
printf("Exited early\n");
fflush(stdout);
}
Sleep(1000);
assert(washere == 2);
return 0;
}
