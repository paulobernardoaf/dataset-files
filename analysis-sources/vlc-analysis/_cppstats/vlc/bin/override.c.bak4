



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdbool.h>
#define MAX_ERRORS 5

void vlc_enable_override (void);

#if defined (__GNUC__) && (defined (__ELF__) && !defined (__sun__))



#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#if defined(HAVE_EXECINFO_H)
#include <execinfo.h>
#include <unistd.h>
#endif
#if defined(NDEBUG)
#undef HAVE_BACKTRACE
#endif

static bool override = false;

static void vlc_reset_override (void)
{
override = false;
}

void vlc_enable_override (void)
{
override = true;
pthread_atfork (NULL, NULL, vlc_reset_override);
}

static void vlogbug (unsigned *pc, const char *level, const char *func,
const char *fmt, va_list ap)
{
#if defined(HAVE_BACKTRACE)
const size_t framec = 5;
void *framev[framec];

backtrace (framev, framec);
#endif
flockfile (stderr);
if (*pc < MAX_ERRORS)
{
(*pc)++;
fprintf (stderr, "%s: call to %s(", level, func);
vfprintf (stderr, fmt, ap);
fputs (")\n", stderr);
fflush (stderr);
#if defined(HAVE_BACKTRACE)
backtrace_symbols_fd (framev + 2, framec - 2, STDERR_FILENO);
#endif
}
funlockfile (stderr);
}

static void logbug (unsigned *pc, const char *level, const char *func,
const char *fmt, ...)
{
va_list ap;

va_start (ap, fmt);
vlogbug (pc, level, func, fmt, ap);
va_end (ap);
}

static void *getsym (const char *name)
{
void *sym = dlsym (RTLD_NEXT, name);
if (sym == NULL)
{
fprintf (stderr, "Cannot resolve symbol %s: %s\n", name,
dlerror ());
abort ();
}
return sym;
}

#define LOG(level, ...) do { static unsigned counter = 0; logbug(&counter, level, __func__, __VA_ARGS__); } while (0)









#define CALL(func, ...) ({ typeof (func) *sym = getsym ( #func); sym (__VA_ARGS__); })










int putenv (char *str)
{
if (override)
{
LOG("Blocked", "\"%s\"", str);
return 0;
}
return CALL(putenv, str);
}

int setenv (const char *name, const char *value, int overwrite)
{
if (override)
{
LOG("Blocked", "\"%s\", \"%s\", %d", name, value, overwrite);
return 0;
}
return CALL(setenv, name, value, overwrite);
}

int unsetenv (const char *name)
{
if (override)
{
LOG("Blocked", "\"%s\"", name);
return 0;
}
return CALL(unsetenv, name);
}










static struct
{
pthread_mutex_t lock;
unsigned int seed;
} prng = { PTHREAD_MUTEX_INITIALIZER, 0, };

void srand (unsigned int seed)
{
pthread_mutex_lock (&prng.lock);
LOG("Warning", "%u", seed);
prng.seed = seed;
pthread_mutex_unlock (&prng.lock);
}

int rand (void)
{
int ret;

pthread_mutex_lock (&prng.lock);
LOG("Warning", "");
ret = rand_r (&prng.seed);
pthread_mutex_unlock (&prng.lock);
return ret;
}


#if 0

#include <signal.h>

static bool blocked_signal (int num)
{
switch (num)
{
case SIGINT:
case SIGHUP:
case SIGQUIT:
case SIGTERM:
case SIGPIPE:
case SIGCHLD:
return true;
}
return false;
}

void (*signal (int signum, void (*handler) (int))) (int)
{
if (override)
{
if (handler != SIG_IGN && handler != SIG_DFL)
goto error;
if (!blocked_signal (signum))
goto error;

if (handler == SIG_DFL)
LOG("Warning", "%d, SIG_DFL", signum, handler);
}
return CALL(signal, signum, handler);
error:
LOG("Blocked", "%d, %p", signum, handler);
return SIG_DFL;
}

int sigaction (int signum, const struct sigaction *act, struct sigaction *old)
{
if (override && act != NULL)
{
if ((act->sa_flags & SA_SIGINFO)
|| (act->sa_handler != SIG_IGN && act->sa_handler != SIG_DFL))
goto error;
if (act->sa_handler == SIG_DFL)
LOG("Warning", "%d, %p, SIG_DFL", signum, act);
}
return CALL(sigaction, signum, act, old);
error:
LOG("Blocked", "%d, %p, %p", signum, act, old);
return -1;
}
#endif







#include <locale.h>

char *setlocale (int cat, const char *locale)
{
if (override && locale != NULL)
{
LOG("Blocked", "%d, \"%s\"", cat, locale);
return NULL;
}
return CALL(setlocale, cat, locale);
}




char *strerror (int val)
{
if (override)
{
static const char msg[] =
"Error message unavailable (use strerror_r instead of strerror)!";
LOG("Blocked", "%d", val);
return (char *)msg;
}
return CALL(strerror, val);
}


#if defined(HAVE_X11_XLIB_H)
#include <X11/Xlib.h>

static pthread_mutex_t xlib_lock = PTHREAD_MUTEX_INITIALIZER;

int (*XSetErrorHandler (int (*handler) (Display *, XErrorEvent *)))
(Display *, XErrorEvent *)
{
if (override)
{
int (*ret) (Display *, XErrorEvent *);

pthread_mutex_lock (&xlib_lock);
LOG("Error", "%p", handler);
ret = CALL(XSetErrorHandler, handler);
pthread_mutex_unlock (&xlib_lock);
return ret;
}
return CALL(XSetErrorHandler, handler);
}

int (*XSetIOErrorHandler (int (*handler) (Display *))) (Display *)
{
if (override)
{
int (*ret) (Display *);

pthread_mutex_lock (&xlib_lock);
LOG("Error", "%p", handler);
ret = CALL(XSetIOErrorHandler, handler);
pthread_mutex_unlock (&xlib_lock);
return ret;
}
return CALL(XSetIOErrorHandler, handler);
}
#endif
#else
void vlc_enable_override (void)
{
}
#endif
