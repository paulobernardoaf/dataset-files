#include "libvlc_internal.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <vlc/libvlc.h>
static const char oom[] = "Out of memory";
static vlc_threadvar_t context;
static char *get_error (void)
{
return vlc_threadvar_get (context);
}
static void free_msg (void *msg)
{
if (msg != oom)
free (msg);
}
static void free_error (void)
{
free_msg (get_error ());
}
static vlc_mutex_t lock = VLC_STATIC_MUTEX;
static uintptr_t refs = 0;
void libvlc_threads_init (void)
{
vlc_mutex_lock (&lock);
if (refs++ == 0)
vlc_threadvar_create (&context, free_msg);
vlc_mutex_unlock (&lock);
}
void libvlc_threads_deinit (void)
{
vlc_mutex_lock (&lock);
assert (refs > 0);
if (--refs == 0)
{
free_error ();
vlc_threadvar_delete (&context);
}
vlc_mutex_unlock (&lock);
}
const char *libvlc_errmsg (void)
{
return get_error ();
}
void libvlc_clearerr (void)
{
free_error ();
vlc_threadvar_set (context, NULL);
}
const char *libvlc_vprinterr (const char *fmt, va_list ap)
{
char *msg;
assert (fmt != NULL);
if (vasprintf (&msg, fmt, ap) == -1)
msg = (char *)oom;
free_error ();
vlc_threadvar_set (context, msg);
return msg;
}
const char *libvlc_printerr (const char *fmt, ...)
{
va_list ap;
const char *msg;
va_start (ap, fmt);
msg = libvlc_vprinterr (fmt, ap);
va_end (ap);
return msg;
}
