


#include <assert.h>
#include <stdbool.h>

#include <uv.h>
#if !defined(WIN32)
#include <signal.h> 
#endif

#include "nvim/ascii.h"
#include "nvim/log.h"
#include "nvim/vim.h"
#include "nvim/globals.h"
#include "nvim/memline.h"
#include "nvim/eval.h"
#include "nvim/fileio.h"
#include "nvim/main.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/event/signal.h"
#include "nvim/os/signal.h"
#include "nvim/event/loop.h"

static SignalWatcher spipe, shup, squit, sterm, susr1;
#if defined(SIGPWR)
static SignalWatcher spwr;
#endif

static bool rejecting_deadly;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/signal.c.generated.h"
#endif

void signal_init(void)
{
#if !defined(WIN32)


sigset_t mask;
sigemptyset(&mask);
if (pthread_sigmask(SIG_SETMASK, &mask, NULL) != 0) {
ELOG("Could not unblock signals, nvim might behave strangely.");
}
#endif

signal_watcher_init(&main_loop, &spipe, NULL);
signal_watcher_init(&main_loop, &shup, NULL);
signal_watcher_init(&main_loop, &squit, NULL);
signal_watcher_init(&main_loop, &sterm, NULL);
#if defined(SIGPIPE)
signal_watcher_start(&spipe, on_signal, SIGPIPE);
#endif
signal_watcher_start(&shup, on_signal, SIGHUP);
#if defined(SIGQUIT)
signal_watcher_start(&squit, on_signal, SIGQUIT);
#endif
signal_watcher_start(&sterm, on_signal, SIGTERM);
#if defined(SIGPWR)
signal_watcher_init(&main_loop, &spwr, NULL);
signal_watcher_start(&spwr, on_signal, SIGPWR);
#endif
#if defined(SIGUSR1)
signal_watcher_init(&main_loop, &susr1, NULL);
signal_watcher_start(&susr1, on_signal, SIGUSR1);
#endif
}

void signal_teardown(void)
{
signal_stop();
signal_watcher_close(&spipe, NULL);
signal_watcher_close(&shup, NULL);
signal_watcher_close(&squit, NULL);
signal_watcher_close(&sterm, NULL);
#if defined(SIGPWR)
signal_watcher_close(&spwr, NULL);
#endif
#if defined(SIGUSR1)
signal_watcher_close(&susr1, NULL);
#endif
}

void signal_stop(void)
{
signal_watcher_stop(&spipe);
signal_watcher_stop(&shup);
signal_watcher_stop(&squit);
signal_watcher_stop(&sterm);
#if defined(SIGPWR)
signal_watcher_stop(&spwr);
#endif
#if defined(SIGUSR1)
signal_watcher_stop(&susr1);
#endif
}

void signal_reject_deadly(void)
{
rejecting_deadly = true;
}

void signal_accept_deadly(void)
{
rejecting_deadly = false;
}

static char * signal_name(int signum)
{
switch (signum) {
#if defined(SIGPWR)
case SIGPWR:
return "SIGPWR";
#endif
#if defined(SIGPIPE)
case SIGPIPE:
return "SIGPIPE";
#endif
case SIGTERM:
return "SIGTERM";
#if defined(SIGQUIT)
case SIGQUIT:
return "SIGQUIT";
#endif
case SIGHUP:
return "SIGHUP";
#if defined(SIGUSR1)
case SIGUSR1:
return "SIGUSR1";
#endif
default:
return "Unknown";
}
}






static void deadly_signal(int signum)
{

set_vim_var_nr(VV_DYING, 1);

WLOG("got signal %d (%s)", signum, signal_name(signum));

snprintf((char *)IObuff, sizeof(IObuff), "Vim: Caught deadly signal '%s'\n",
signal_name(signum));


preserve_exit();
}

static void on_signal(SignalWatcher *handle, int signum, void *data)
{
assert(signum >= 0);
switch (signum) {
#if defined(SIGPWR)
case SIGPWR:


ml_sync_all(false, false, true);
break;
#endif
#if defined(SIGPIPE)
case SIGPIPE:

break;
#endif
case SIGTERM:
#if defined(SIGQUIT)
case SIGQUIT:
#endif
case SIGHUP:
if (!rejecting_deadly) {
deadly_signal(signum);
}
break;
#if defined(SIGUSR1)
case SIGUSR1:
apply_autocmds(EVENT_SIGNAL, (char_u *)"SIGUSR1", curbuf->b_fname, true,
curbuf);
break;
#endif
default:
ELOG("invalid signal: %d", signum);
break;
}
}
