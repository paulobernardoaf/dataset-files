
























#if !defined(VLC_DBUS_DBUS_PLAYER_H_)
#define VLC_DBUS_DBUS_PLAYER_H_

#include <vlc_interface.h>
#include "dbus_common.h"

#define DBUS_MPRIS_PLAYER_INTERFACE "org.mpris.MediaPlayer2.Player"

#define LOOP_STATUS_NONE "None"
#define LOOP_STATUS_TRACK "Track"
#define LOOP_STATUS_PLAYLIST "Playlist"

#define PLAYBACK_STATUS_STOPPED "Stopped"
#define PLAYBACK_STATUS_PLAYING "Playing"
#define PLAYBACK_STATUS_PAUSED "Paused"


DBusHandlerResult handle_player ( DBusConnection *p_conn,
DBusMessage *p_from,
void *p_this );


enum
{
PLAYER_CAPS_NONE = 0,
PLAYER_CAN_GO_NEXT = 1 << 0,
PLAYER_CAN_GO_PREVIOUS = 1 << 1,
PLAYER_CAN_PAUSE = 1 << 2,
PLAYER_CAN_PLAY = 1 << 3,
PLAYER_CAN_SEEK = 1 << 4,
PLAYER_CAN_PROVIDE_METADATA = 1 << 5,
PLAYER_CAN_PROVIDE_POSITION = 1 << 6,
PLAYER_CAN_REPEAT = 1 << 7,
PLAYER_CAN_LOOP = 1 << 8,
PLAYER_CAN_SHUFFLE = 1 << 9,
PLAYER_CAN_CONTROL_RATE = 1 << 10,
PLAYER_CAN_PLAY_BACKWARDS = 1 << 11
};

int PlayerStatusChangedEmit ( intf_thread_t * );
int PlayerCapsChangedEmit ( intf_thread_t * );
int PlayerMetadataChangedEmit( intf_thread_t*, input_item_t* );
int TrackChangedEmit ( intf_thread_t *, input_item_t * );
int SeekedEmit( intf_thread_t * );

int PlayerPropertiesChangedEmit( intf_thread_t *, vlc_dictionary_t * );

void UpdatePlayerCaps( intf_thread_t * );

#endif 
