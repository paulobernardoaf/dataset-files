



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#if defined(_WIN32)
#include <windows.h>
#endif

pid_t getpid (void)
{
#if defined (_WIN32)
return (pid_t) GetCurrentProcessId ();
#elif defined (__native_client__)
return 1;
#else
#error Unimplemented!
#endif
}
