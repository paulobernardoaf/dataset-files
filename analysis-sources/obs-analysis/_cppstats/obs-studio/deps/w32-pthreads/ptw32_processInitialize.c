#include "pthread.h"
#include "implement.h"
int
ptw32_processInitialize (void)
{
if (ptw32_processInitialized)
{
return PTW32_TRUE;
}
ptw32_processInitialized = PTW32_TRUE;
if ((pthread_key_create (&ptw32_selfThreadKey, NULL) != 0) ||
(pthread_key_create (&ptw32_cleanupKey, NULL) != 0))
{
ptw32_processTerminate ();
}
return (ptw32_processInitialized);
} 
