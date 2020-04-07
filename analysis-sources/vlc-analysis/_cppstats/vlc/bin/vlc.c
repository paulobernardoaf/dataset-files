#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc/vlc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif
#include <unistd.h>
#if defined(__OS2__)
#include <iconv.h>
#define pthread_t int
#define pthread_self() _gettid()
static char *FromSystem(const void *str)
{
iconv_t handle = iconv_open ("UTF-8", "");
if (handle == (iconv_t)(-1))
return NULL;
size_t str_len = strlen (str);
char *out = NULL;
for (unsigned mul = 4; mul < 8; mul++)
{
size_t in_size = str_len;
const char *in = str;
size_t out_max = mul * str_len;
char *tmp = out = malloc (1 + out_max);
if (!out)
break;
if (iconv (handle, &in, &in_size, &tmp, &out_max) != (size_t)(-1)) {
*tmp = '\0';
break;
}
free(out);
out = NULL;
if (errno != E2BIG)
break;
}
iconv_close(handle);
return out;
}
#endif
extern void vlc_enable_override (void);
static bool signal_ignored (int signum)
{
struct sigaction sa;
if (sigaction (signum, NULL, &sa))
return false;
return ((sa.sa_flags & SA_SIGINFO)
? (void *)sa.sa_sigaction : (void *)sa.sa_handler) == SIG_IGN;
}
static void vlc_kill (void *data)
{
#if !defined(__OS2__)
pthread_t *ps = data;
pthread_kill (*ps, SIGTERM);
#else
kill (getpid(), SIGTERM);
#endif
}
static void exit_timeout (int signum)
{
(void) signum;
signal (SIGINT, SIG_DFL);
}
int main(int argc, const char *argv[])
{
signal (SIGPIPE, SIG_IGN);
signal (SIGCHLD, SIG_DFL);
#if !defined(NDEBUG)
setenv ("MALLOC_CHECK_", "2", 1);
setenv ("GNOME_DISABLE_CRASH_DIALOG", "1", 1);
#endif
#if defined(TOP_BUILDDIR)
setenv ("VLC_PLUGIN_PATH", TOP_BUILDDIR"/modules", 1);
setenv ("VLC_DATA_PATH", TOP_SRCDIR"/share", 1);
setenv ("VLC_LIB_PATH", TOP_BUILDDIR"/modules", 1);
#endif
unsetenv ("DESKTOP_STARTUP_ID");
#if !defined(ALLOW_RUN_AS_ROOT)
if (geteuid () == 0)
{
fprintf (stderr, "VLC is not supposed to be run as root. Sorry.\n"
"If you need to use real-time priorities and/or privileged TCP ports\n"
"you can use %s-wrapper (make sure it is Set-UID root and\n"
"cannot be run by non-trusted users first).\n", argv[0]);
return 1;
}
#endif
setlocale (LC_ALL, "");
if (isatty (STDERR_FILENO))
fprintf (stderr, "VLC media player %s (revision %s)\n",
libvlc_get_version(), libvlc_get_changeset());
sigset_t set;
sigemptyset (&set);
sigaddset (&set, SIGINT);
sigaddset (&set, SIGHUP);
sigaddset (&set, SIGQUIT);
sigaddset (&set, SIGTERM);
sigaddset (&set, SIGPIPE);
sigaddset (&set, SIGCHLD);
pthread_t self = pthread_self ();
pthread_sigmask (SIG_SETMASK, &set, NULL);
const char *args[argc + 3];
int count = 0;
args[count++] = "--no-ignore-config";
args[count++] = "--media-library";
#if defined(HAVE_DBUS)
args[count++] = "--dbus";
#endif
#if defined(__OS2__)
for (int i = 1; i < argc; i++)
if ((args[count++] = FromSystem(argv[i])) == NULL)
{
fprintf (stderr, "Converting '%s' to UTF-8 failed.\n", argv[i]);
return 1;
}
#else
memcpy(args + count, argv + 1, (argc - 1) * sizeof (*argv));
count += (argc - 1);
#endif
args[count] = NULL;
vlc_enable_override ();
libvlc_instance_t *vlc = libvlc_new(count, args);
if (vlc == NULL)
return 1;
int ret = 1;
libvlc_set_exit_handler (vlc, vlc_kill, &self);
libvlc_set_app_id (vlc, "org.VideoLAN.VLC", PACKAGE_VERSION, PACKAGE_NAME);
libvlc_set_user_agent (vlc, "VLC media player", "VLC/"PACKAGE_VERSION);
libvlc_add_intf (vlc, "hotkeys,none");
#if !defined (__OS2__)
libvlc_add_intf (vlc, "globalhotkeys,none");
#endif
if (libvlc_add_intf (vlc, NULL))
{
fprintf(stderr, "%s: cannot start any interface. Exiting.\n", argv[0]);
goto out;
}
libvlc_playlist_play (vlc);
sigdelset (&set, SIGCHLD);
pthread_sigmask (SIG_SETMASK, &set, NULL);
if (signal_ignored (SIGHUP))
sigdelset (&set, SIGHUP);
sigdelset (&set, SIGPIPE);
int signum;
sigwait (&set, &signum);
sigemptyset (&set);
sigaddset (&set, SIGINT);
sigaddset (&set, SIGALRM);
signal (SIGINT, SIG_IGN);
signal (SIGALRM, exit_timeout);
pthread_sigmask (SIG_UNBLOCK, &set, NULL);
alarm (3);
ret = 0;
out:
libvlc_release (vlc);
#if defined(__OS2__)
for (int i = count - argc + 1; i < count; i++)
free(args[i]);
#endif
return ret;
}
