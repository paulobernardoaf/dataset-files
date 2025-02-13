#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../libvlc.h"
#include "../lib/libvlc_internal.h"
#if defined(HAVE_DBUS)
#include <dbus/dbus.h>
#include <vlc_url.h>
#endif
void system_Init (void)
{
}
static void system_ConfigureDbus(libvlc_int_t *vlc, int argc,
const char *const argv[])
{
#if defined(HAVE_DBUS)
#define MPRIS_APPEND "/org/mpris/MediaPlayer2/TrackList/Append"
#define MPRIS_BUS_NAME "org.mpris.MediaPlayer2.vlc"
#define MPRIS_OBJECT_PATH "/org/mpris/MediaPlayer2"
#define MPRIS_TRACKLIST_INTERFACE "org.mpris.MediaPlayer2.TrackList"
dbus_threads_init_default();
if (var_InheritBool(vlc, "dbus"))
libvlc_InternalAddIntf(vlc, "dbus,none");
if (!var_InheritBool(vlc, "one-instance")
&& !(var_InheritBool(vlc, "one-instance-when-started-from-file")
&& var_InheritBool(vlc, "started-from-file")))
return;
for (int i = 0; i < argc; i++)
if (argv[i][0] == ':')
{
msg_Err(vlc, "item option %s incompatible with single instance",
argv[i]);
return;
}
char *name = var_GetString(vlc, "dbus-mpris-name");
if (name != NULL)
{
bool singleton = !strcmp(name, MPRIS_BUS_NAME);
free(name);
if (singleton)
{
msg_Dbg(vlc, "no running VLC instance - continuing normally...");
return; 
}
}
DBusError err;
dbus_error_init(&err);
DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
if (conn == NULL)
{
msg_Err(vlc, "D-Bus session bus connection failure: %s",
err.message);
dbus_error_free(&err);
return;
}
msg_Warn(vlc, "running VLC instance - exiting...");
const dbus_bool_t play = !var_InheritBool(vlc, "playlist-enqueue");
for (int i = 0; i < argc; i++)
{
DBusMessage *req = dbus_message_new_method_call(MPRIS_BUS_NAME,
MPRIS_OBJECT_PATH, MPRIS_TRACKLIST_INTERFACE, "AddTrack");
if (unlikely(req == NULL))
continue;
char *mrlbuf = NULL;
const char *mrl;
if (strstr(argv[i], "://"))
mrl = argv[i];
else
{
mrlbuf = vlc_path2uri(argv[i], NULL);
if (unlikely(mrlbuf == NULL))
{
dbus_message_unref(req);
continue;
}
mrl = mrlbuf;
}
msg_Dbg(vlc, "adding track %s to running instance", mrl);
const char *after_track = MPRIS_APPEND;
dbus_bool_t ok = dbus_message_append_args(req, DBUS_TYPE_STRING, &mrl,
DBUS_TYPE_OBJECT_PATH, &after_track,
DBUS_TYPE_BOOLEAN, &play,
DBUS_TYPE_INVALID);
free(mrlbuf);
if (unlikely(!ok))
{
dbus_message_unref(req);
continue;
}
DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn,
req, -1, &err);
dbus_message_unref(req);
if (reply == NULL)
{
msg_Err(vlc, "D-Bus error: %s", err.message);
dbus_error_free(&err);
continue;
}
dbus_message_unref(reply);
}
dbus_connection_unref(conn);
exit(0);
#else
(void) vlc; (void) argc; (void) argv;
#endif 
}
void system_Configure(libvlc_int_t *libvlc,
int argc, const char *const argv[])
{
system_ConfigureDbus(libvlc, argc, argv);
}
