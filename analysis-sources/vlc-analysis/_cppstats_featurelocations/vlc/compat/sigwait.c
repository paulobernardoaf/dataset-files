





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(__native_client__)




#include <assert.h>
#include <signal.h>

int sigwait(const sigset_t *set, int *sig)
{
sigset_t s = *set;
if (sigemptyset(&s))
return 0;
assert(sigismember(&s, SIGPIPE));
sigdelset(&s, SIGPIPE);
assert(sigemptyset(&s));

*sig = SIGPIPE;
return 0;
}
#else
#error sigwait not implemented on your platform!
#endif
