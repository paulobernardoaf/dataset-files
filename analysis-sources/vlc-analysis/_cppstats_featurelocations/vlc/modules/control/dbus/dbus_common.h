


























#if !defined(VLC_DBUS_DBUS_COMMON_H_)
#define VLC_DBUS_DBUS_COMMON_H_

#include <vlc_common.h>
#include <vlc_interface.h>
#include <vlc_player.h>
#include <vlc_playlist.h>
#include <dbus/dbus.h>

#define DBUS_MPRIS_OBJECT_PATH "/org/mpris/MediaPlayer2"



#define INTF ((intf_thread_t *)p_this)
#define PL (INTF->p_sys->playlist)

#define DBUS_METHOD( method_function ) static DBusHandlerResult method_function ( DBusConnection *p_conn, DBusMessage *p_from, void *p_this )



#define DBUS_SIGNAL( signal_function ) static DBusHandlerResult signal_function ( DBusConnection *p_conn, void *p_data )



#define REPLY_INIT DBusMessage* p_msg = dbus_message_new_method_return( p_from ); if( !p_msg ) return DBUS_HANDLER_RESULT_NEED_MEMORY; 



#define REPLY_SEND if( !dbus_connection_send( p_conn, p_msg, NULL ) ) return DBUS_HANDLER_RESULT_NEED_MEMORY; dbus_connection_flush( p_conn ); dbus_message_unref( p_msg ); return DBUS_HANDLER_RESULT_HANDLED






#define SIGNAL_INIT( interface, path, signal ) DBusMessage *p_msg = dbus_message_new_signal( path, interface, signal ); if( !p_msg ) return DBUS_HANDLER_RESULT_NEED_MEMORY; 




#define SIGNAL_SEND if( !dbus_connection_send( p_conn, p_msg, NULL ) ) return DBUS_HANDLER_RESULT_NEED_MEMORY; dbus_message_unref( p_msg ); dbus_connection_flush( p_conn ); return DBUS_HANDLER_RESULT_HANDLED






#define OUT_ARGUMENTS DBusMessageIter args; dbus_message_iter_init_append( p_msg, &args )



#define DBUS_ADD( dbus_type, value ) if( !dbus_message_iter_append_basic( &args, dbus_type, value ) ) return DBUS_HANDLER_RESULT_NEED_MEMORY



#define ADD_STRING( s ) DBUS_ADD( DBUS_TYPE_STRING, s )
#define ADD_DOUBLE( d ) DBUS_ADD( DBUS_TYPE_DOUBLE, d )
#define ADD_BOOL( b ) DBUS_ADD( DBUS_TYPE_BOOLEAN, b )
#define ADD_INT32( i ) DBUS_ADD( DBUS_TYPE_INT32, i )
#define ADD_INT64( i ) DBUS_ADD( DBUS_TYPE_INT64, i )
#define ADD_BYTE( b ) DBUS_ADD( DBUS_TYPE_BYTE, b )

#define MPRIS_TRACKID_FORMAT "/org/videolan/vlc/playlist/%lu"

struct intf_sys_t
{
vlc_playlist_t *playlist;
vlc_playlist_listener_id *playlist_listener;
vlc_player_listener_id *player_listener;
vlc_player_aout_listener_id *player_aout_listener;
vlc_player_vout_listener_id *player_vout_listener;

DBusConnection *p_conn;
bool b_meta_read;
dbus_int32_t i_player_caps;
dbus_int32_t i_playing_state;
bool b_can_play;
bool b_dead;
vlc_array_t events;
vlc_array_t timeouts;
vlc_array_t watches;
int p_pipe_fds[2];
vlc_mutex_t lock;
vlc_thread_t thread;
bool has_input;

vlc_tick_t i_last_input_pos; 
vlc_tick_t i_last_input_pos_event; 
};

enum
{
SIGNAL_NONE=0,
SIGNAL_ITEM_CURRENT,
SIGNAL_PLAYLIST_ITEM_APPEND,
SIGNAL_PLAYLIST_ITEM_DELETED,
SIGNAL_INPUT_METADATA,
SIGNAL_RANDOM,
SIGNAL_REPEAT,
SIGNAL_LOOP,
SIGNAL_STATE,
SIGNAL_RATE,
SIGNAL_SEEK,
SIGNAL_CAN_SEEK,
SIGNAL_CAN_PAUSE,
SIGNAL_VOLUME_CHANGE,
SIGNAL_VOLUME_MUTED,
SIGNAL_FULLSCREEN
};

enum
{
PLAYBACK_STATE_INVALID = -1,
PLAYBACK_STATE_PLAYING = 0,
PLAYBACK_STATE_PAUSED = 1,
PLAYBACK_STATE_STOPPED = 2
};

int DemarshalSetPropertyValue( DBusMessage *p_msg, void *p_arg );
int GetInputMeta( vlc_playlist_t *, vlc_playlist_item_t *,
DBusMessageIter *args );
int AddProperty ( intf_thread_t *p_intf,
DBusMessageIter *p_container,
const char* psz_property_name,
const char* psz_signature,
int (*pf_marshaller) (intf_thread_t*, DBusMessageIter*) );

#endif 
