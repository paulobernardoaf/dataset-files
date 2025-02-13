
























#if !defined(VLC_DBUS_DBUS_TRACKLIST_H_)
#define VLC_DBUS_DBUS_TRACKLIST_H_

#include <vlc_common.h>
#include <vlc_interface.h>
#include "dbus_common.h"

#define DBUS_MPRIS_TRACKLIST_INTERFACE "org.mpris.MediaPlayer2.TrackList"
#define DBUS_MPRIS_TRACKLIST_PATH "/org/mpris/MediaPlayer2/TrackList"

#define DBUS_MPRIS_NOTRACK "/org/mpris/MediaPlayer2/TrackList/NoTrack"
#define DBUS_MPRIS_APPEND "/org/mpris/MediaPlayer2/TrackList/Append"


DBusHandlerResult handle_tracklist ( DBusConnection *p_conn,
DBusMessage *p_from,
void *p_this );

int TrackListPropertiesChangedEmit( intf_thread_t *, vlc_dictionary_t * );

#endif 
