#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <pulse/pulseaudio.h>
#include "audio_output/vlcpulse.h"
#include <assert.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <pwd.h>
const char vlc_module_name[] = "vlcpulse";
#undef vlc_pa_error
void vlc_pa_error (vlc_object_t *obj, const char *msg, pa_context *ctx)
{
msg_Err (obj, "%s: %s", msg, pa_strerror (pa_context_errno (ctx)));
}
static void context_state_cb (pa_context *ctx, void *userdata)
{
pa_threaded_mainloop *mainloop = userdata;
switch (pa_context_get_state(ctx))
{
case PA_CONTEXT_READY:
case PA_CONTEXT_FAILED:
case PA_CONTEXT_TERMINATED:
pa_threaded_mainloop_signal (mainloop, 0);
default:
break;
}
}
static bool context_wait (pa_context *ctx, pa_threaded_mainloop *mainloop)
{
pa_context_state_t state;
while ((state = pa_context_get_state (ctx)) != PA_CONTEXT_READY)
{
if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED)
return -1;
pa_threaded_mainloop_wait (mainloop);
}
return 0;
}
static void context_event_cb(pa_context *c, const char *name, pa_proplist *pl,
void *userdata)
{
vlc_object_t *obj = userdata;
msg_Warn (obj, "unhandled context event \"%s\"", name);
(void) c;
(void) pl;
}
pa_context *vlc_pa_connect (vlc_object_t *obj, pa_threaded_mainloop **mlp)
{
msg_Dbg (obj, "using library version %s", pa_get_library_version ());
msg_Dbg (obj, " (compiled with version %s, protocol %u)",
pa_get_headers_version (), PA_PROTOCOL_VERSION);
pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new ();
if (unlikely(mainloop == NULL))
return NULL;
if (pa_threaded_mainloop_start (mainloop) < 0)
{
pa_threaded_mainloop_free (mainloop);
return NULL;
}
char *ua = var_InheritString (obj, "user-agent");
pa_proplist *props = pa_proplist_new ();
if (likely(props != NULL))
{
char *str;
if (ua != NULL)
pa_proplist_sets (props, PA_PROP_APPLICATION_NAME, ua);
str = var_InheritString (obj, "app-id");
if (str != NULL)
{
pa_proplist_sets (props, PA_PROP_APPLICATION_ID, str);
free (str);
}
str = var_InheritString (obj, "app-version");
if (str != NULL)
{
pa_proplist_sets (props, PA_PROP_APPLICATION_VERSION, str);
free (str);
}
str = var_InheritString (obj, "app-icon-name");
if (str != NULL)
{
pa_proplist_sets (props, PA_PROP_APPLICATION_ICON_NAME, str);
free (str);
}
pa_proplist_sets (props, PA_PROP_APPLICATION_LANGUAGE,
setlocale (LC_MESSAGES, NULL));
pa_proplist_setf (props, PA_PROP_APPLICATION_PROCESS_ID, "%lu",
(unsigned long) getpid ());
for (size_t max = sysconf (_SC_GETPW_R_SIZE_MAX), len = max % 1024 + 1024;
len < max; len += 1024)
{
struct passwd pwbuf, *pw;
char buf[len];
if (getpwuid_r (getuid (), &pwbuf, buf, sizeof (buf), &pw) == 0)
{
if (pw != NULL)
pa_proplist_sets (props, PA_PROP_APPLICATION_PROCESS_USER,
pw->pw_name);
break;
}
}
for (size_t max = sysconf (_SC_HOST_NAME_MAX), len = max % 1024 + 1024;
len < max; len += 1024)
{
char hostname[len];
if (gethostname (hostname, sizeof (hostname)) == 0)
{
pa_proplist_sets (props, PA_PROP_APPLICATION_PROCESS_HOST,
hostname);
break;
}
}
const char *session = getenv ("XDG_SESSION_COOKIE");
if (session != NULL)
{
pa_proplist_setf (props, PA_PROP_APPLICATION_PROCESS_MACHINE_ID,
"%.32s", session); 
pa_proplist_sets (props, PA_PROP_APPLICATION_PROCESS_SESSION_ID,
session);
}
}
pa_context *ctx;
pa_mainloop_api *api;
pa_threaded_mainloop_lock (mainloop);
api = pa_threaded_mainloop_get_api (mainloop);
ctx = pa_context_new_with_proplist (api, ua, props);
free (ua);
if (props != NULL)
pa_proplist_free (props);
if (unlikely(ctx == NULL))
goto fail;
pa_context_set_state_callback (ctx, context_state_cb, mainloop);
pa_context_set_event_callback (ctx, context_event_cb, obj);
if (pa_context_connect (ctx, NULL, 0, NULL) < 0
|| context_wait (ctx, mainloop))
{
vlc_pa_error (obj, "PulseAudio server connection failure", ctx);
pa_context_unref (ctx);
goto fail;
}
msg_Dbg (obj, "connected %s to %s as client #%"PRIu32,
pa_context_is_local (ctx) ? "locally" : "remotely",
pa_context_get_server (ctx), pa_context_get_index (ctx));
msg_Dbg (obj, "using protocol %"PRIu32", server protocol %"PRIu32,
pa_context_get_protocol_version (ctx),
pa_context_get_server_protocol_version (ctx));
pa_threaded_mainloop_unlock (mainloop);
*mlp = mainloop;
return ctx;
fail:
pa_threaded_mainloop_unlock (mainloop);
pa_threaded_mainloop_stop (mainloop);
pa_threaded_mainloop_free (mainloop);
return NULL;
}
void vlc_pa_disconnect (vlc_object_t *obj, pa_context *ctx,
pa_threaded_mainloop *mainloop)
{
pa_threaded_mainloop_lock (mainloop);
pa_context_disconnect (ctx);
pa_context_set_event_callback (ctx, NULL, NULL);
pa_context_set_state_callback (ctx, NULL, NULL);
pa_context_unref (ctx);
pa_threaded_mainloop_unlock (mainloop);
pa_threaded_mainloop_stop (mainloop);
pa_threaded_mainloop_free (mainloop);
(void) obj;
}
void vlc_pa_rttime_free (pa_threaded_mainloop *mainloop, pa_time_event *e)
{
pa_mainloop_api *api = pa_threaded_mainloop_get_api (mainloop);
api->time_free (e);
}
#undef vlc_pa_get_latency
vlc_tick_t vlc_pa_get_latency(vlc_object_t *obj, pa_context *ctx, pa_stream *s)
{
const pa_sample_spec *ss = pa_stream_get_sample_spec(s);
const pa_timing_info *ti = pa_stream_get_timing_info(s);
if (ti == NULL) {
msg_Dbg(obj, "no timing infos");
return VLC_TICK_INVALID;
}
if (ti->write_index_corrupt) {
msg_Dbg(obj, "write index corrupt");
return VLC_TICK_INVALID;
}
pa_usec_t wt = pa_bytes_to_usec((uint64_t)ti->write_index, ss);
pa_usec_t rt;
if (pa_stream_get_time(s, &rt)) {
if (pa_context_errno(ctx) != PA_ERR_NODATA)
vlc_pa_error(obj, "unknown time", ctx);
return VLC_TICK_INVALID;
}
union { uint64_t u; int64_t s; } d;
d.u = wt - rt;
return d.s; 
}
