#include "dbus_common.h"
static const char* psz_introspection_xml =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
" <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
" <method name=\"Introspect\">\n"
" <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
" </method>\n"
" </interface>\n"
" <interface name=\"org.freedesktop.DBus.Properties\">\n"
" <method name=\"Get\">\n"
" <arg direction=\"in\" type=\"s\"/>\n"
" <arg direction=\"in\" type=\"s\"/>\n"
" <arg direction=\"out\" type=\"v\"/>\n"
" </method>\n"
" <method name=\"Set\">\n"
" <arg direction=\"in\" type=\"s\"/>\n"
" <arg direction=\"in\" type=\"s\"/>\n"
" <arg direction=\"in\" type=\"v\"/>\n"
" </method>\n"
" <method name=\"GetAll\">\n"
" <arg direction=\"in\" type=\"s\"/>\n"
" <arg direction=\"out\" type=\"a{sv}\"/>\n"
" </method>\n"
" <signal name=\"PropertiesChanged\">\n"
" <arg type=\"s\"/>\n"
" <arg type=\"a{sv}\"/>\n"
" <arg type=\"as\"/>\n"
" </signal>\n"
" </interface>\n"
" <interface name=\"org.mpris.MediaPlayer2\">\n"
" <property name=\"CanQuit\" type=\"b\" access=\"read\" />\n"
" <property name=\"Fullscreen\" type=\"b\" access=\"readwrite\" />\n"
" <property name=\"CanSetFullscreen\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanRaise\" type=\"b\" access=\"read\" />\n"
" <property name=\"HasTrackList\" type=\"b\" access=\"read\" />\n"
" <property name=\"Identity\" type=\"s\" access=\"read\" />\n"
" <property name=\"DesktopEntry\" type=\"s\" access=\"read\" />\n"
" <property name=\"SupportedUriSchemes\" type=\"as\" access=\"read\" />\n"
" <property name=\"SupportedMimeTypes\" type=\"as\" access=\"read\" />\n"
" <method name=\"Raise\" />\n"
" <method name=\"Quit\" />\n"
" </interface>\n"
" <interface name=\"org.mpris.MediaPlayer2.Player\">\n"
" <property name=\"PlaybackStatus\" type=\"s\" access=\"read\" />\n"
" <property name=\"LoopStatus\" type=\"s\" access=\"readwrite\" />\n"
" <property name=\"Rate\" type=\"d\" access=\"readwrite\" />\n"
" <property name=\"Shuffle\" type=\"b\" access=\"readwrite\" />\n"
" <property name=\"Metadata\" type=\"a{sv}\" access=\"read\" />\n"
" <property name=\"Volume\" type=\"d\" access=\"readwrite\" />\n"
" <property name=\"Position\" type=\"x\" access=\"read\" />\n"
" <property name=\"MinimumRate\" type=\"d\" access=\"read\" />\n"
" <property name=\"MaximumRate\" type=\"d\" access=\"read\" />\n"
" <property name=\"CanGoNext\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanGoPrevious\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanPlay\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanPause\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanSeek\" type=\"b\" access=\"read\" />\n"
" <property name=\"CanControl\" type=\"b\" access=\"read\" />\n"
" <method name=\"Next\" />\n"
" <method name=\"Previous\" />\n"
" <method name=\"Pause\" />\n"
" <method name=\"PlayPause\" />\n"
" <method name=\"Stop\" />\n"
" <method name=\"Play\" />\n"
" <method name=\"Seek\">\n"
" <arg type=\"x\" direction=\"in\" />\n"
" </method>"
" <method name=\"SetPosition\">\n"
" <arg type=\"o\" direction=\"in\" />\n"
" <arg type=\"x\" direction=\"in\" />\n"
" </method>\n"
" <method name=\"OpenUri\">\n"
" <arg type=\"s\" direction=\"in\" />\n"
" </method>\n"
" <signal name=\"Seeked\">\n"
" <arg type=\"x\" />\n"
" </signal>\n"
" </interface>\n"
" <interface name=\"org.mpris.MediaPlayer2.TrackList\">\n"
" <property name=\"Tracks\" type=\"ao\" access=\"read\" />\n"
" <property name=\"CanEditTracks\" type=\"b\" access=\"read\" />\n"
" <method name=\"GetTracksMetadata\">\n"
" <arg type=\"ao\" direction=\"in\" />\n"
" <arg type=\"aa{sv}\" direction=\"out\" />\n"
" </method>\n"
" <method name=\"AddTrack\">\n"
" <arg type=\"s\" direction=\"in\" />\n"
" <arg type=\"o\" direction=\"in\" />\n"
" <arg type=\"b\" direction=\"in\" />\n"
" </method>\n"
" <method name=\"RemoveTrack\">\n"
" <arg type=\"o\" direction=\"in\" />\n"
" </method>\n"
" <method name=\"GoTo\">\n"
" <arg type=\"o\" direction=\"in\" />\n"
" </method>\n"
" <signal name=\"TrackListReplaced\">\n"
" <arg type=\"ao\" />\n"
" <arg type=\"o\" />\n"
" </signal>\n"
" <signal name=\"TrackAdded\">\n"
" <arg type=\"a{sv}\" />\n"
" <arg type=\"o\" />\n"
" </signal>\n"
" <signal name=\"TrackRemoved\">\n"
" <arg type=\"o\" />\n"
" </signal>\n"
" <signal name=\"TrackMetadataChanged\">\n"
" <arg type=\"o\" />\n"
" <arg type=\"a{sv}\" />\n"
" </signal>\n"
" </interface>\n"
"</node>\n"
;
DBUS_METHOD( handle_introspect )
{
VLC_UNUSED(p_this);
REPLY_INIT;
OUT_ARGUMENTS;
ADD_STRING( &psz_introspection_xml );
REPLY_SEND;
}
