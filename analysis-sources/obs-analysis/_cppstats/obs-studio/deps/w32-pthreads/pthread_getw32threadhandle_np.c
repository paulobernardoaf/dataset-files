#include "pthread.h"
#include "implement.h"
HANDLE
pthread_getw32threadhandle_np (pthread_t thread)
{
return ((ptw32_thread_t *)thread.p)->threadH;
}
DWORD
pthread_getw32threadid_np (pthread_t thread)
{
return ((ptw32_thread_t *)thread.p)->thread;
}
