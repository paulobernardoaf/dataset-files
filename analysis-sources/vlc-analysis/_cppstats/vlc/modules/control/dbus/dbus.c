#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <dbus/dbus.h>
#include "dbus_common.h"
#include "dbus_root.h"
#include "dbus_player.h"
#include "dbus_tracklist.h"
#include "dbus_introspect.h"
#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_meta.h>
#include <vlc_tick.h>
#include <vlc_fs.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#define DBUS_MPRIS_BUS_NAME "org.mpris.MediaPlayer2.vlc"
#define DBUS_INSTANCE_ID_PREFIX "instance"
#define SEEK_THRESHOLD 1000 
#define EVENTS_DELAY INT64_C(100000) 
static DBusHandlerResult
MPRISEntryPoint ( DBusConnection *p_conn, DBusMessage *p_from, void *p_this );
static const DBusObjectPathVTable dbus_mpris_vtable = {
NULL, MPRISEntryPoint, 
NULL, NULL, NULL, NULL
};
typedef struct
{
int signal;
} callback_info_t;
enum
{
PIPE_OUT = 0,
PIPE_IN = 1
};
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static void *Run ( void * );
static int TrackChange( intf_thread_t * );
static dbus_bool_t add_timeout(DBusTimeout *, void *);
static void remove_timeout(DBusTimeout *, void *);
static void toggle_timeout(DBusTimeout *, void *);
static dbus_bool_t add_watch ( DBusWatch *p_watch, void *p_data );
static void remove_watch ( DBusWatch *p_watch, void *p_data );
static void watch_toggled ( DBusWatch *p_watch, void *p_data );
static void wakeup_main_loop( void *p_data );
static void ProcessEvents ( intf_thread_t *p_intf,
callback_info_t **p_events,
int i_events );
static void ProcessWatches ( intf_thread_t *p_intf,
DBusWatch **p_watches,
int i_watches,
struct pollfd *p_fds,
int i_fds );
static void DispatchDBusMessages( intf_thread_t *p_intf );
static void playlist_on_items_added(vlc_playlist_t *,
size_t, vlc_playlist_item_t *const [],
size_t, void *);
static void playlist_on_items_removed(vlc_playlist_t *,
size_t, size_t, void *);
static void playlist_on_playback_repeat_changed(vlc_playlist_t *,
enum vlc_playlist_playback_repeat,
void *);
static void playlist_on_playback_order_changed(vlc_playlist_t *,
enum vlc_playlist_playback_order,
void *);
static void playlist_on_current_index_changed(vlc_playlist_t *,
ssize_t, void *);
static void player_on_state_changed(vlc_player_t *,
enum vlc_player_state, void *);
static void player_on_error_changed(vlc_player_t *,
enum vlc_player_error, void *);
static void player_on_rate_changed(vlc_player_t *, float, void *);
static void player_on_capabilities_changed(vlc_player_t *, int, int, void *);
static void player_on_position_changed(vlc_player_t *,
vlc_tick_t, float, void *);
static void player_on_media_meta_changed(vlc_player_t *,
input_item_t *, void *);
static void player_aout_on_volume_changed(audio_output_t *, float, void *);
static void player_aout_on_mute_changed(audio_output_t *, bool, void *);
static void player_vout_on_fullscreen_changed(vout_thread_t *, bool, void *);
vlc_module_begin ()
set_shortname( N_("DBus"))
set_category( CAT_INTERFACE )
set_description( N_("D-Bus control interface") )
set_capability( "interface", 0 )
set_callbacks( Open, Close )
vlc_module_end ()
static int Open( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t*)p_this;
vlc_object_t *vlc = VLC_OBJECT(vlc_object_instance(p_this));
vlc_playlist_t *playlist = NULL;
if( !dbus_threads_init_default() )
return VLC_EGENERIC;
intf_sys_t *p_sys = calloc( 1, sizeof( intf_sys_t ) );
if( unlikely(!p_sys) )
return VLC_ENOMEM;
DBusConnection *p_conn;
p_sys->i_player_caps = PLAYER_CAPS_NONE;
p_sys->i_playing_state = PLAYBACK_STATE_INVALID;
if( vlc_pipe( p_sys->p_pipe_fds ) )
{
free( p_sys );
msg_Err( p_intf, "Could not create pipe" );
return VLC_EGENERIC;
}
DBusError error;
dbus_error_init( &error );
p_conn = dbus_bus_get_private( DBUS_BUS_SESSION, &error );
if( !p_conn )
{
msg_Err( p_this, "Failed to connect to the D-Bus session daemon: %s",
error.message );
dbus_error_free( &error );
goto dbus_connection_failure;
}
dbus_connection_set_exit_on_disconnect( p_conn, FALSE );
dbus_connection_register_object_path( p_conn, DBUS_MPRIS_OBJECT_PATH,
&dbus_mpris_vtable, p_this );
const unsigned bus_flags = DBUS_NAME_FLAG_DO_NOT_QUEUE;
var_Create(vlc, "dbus-mpris-name", VLC_VAR_STRING);
if( dbus_bus_request_name( p_conn, DBUS_MPRIS_BUS_NAME, bus_flags, NULL )
!= DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER )
{
char unique_service[sizeof( DBUS_MPRIS_BUS_NAME ) +
sizeof( DBUS_INSTANCE_ID_PREFIX ) + 10];
snprintf( unique_service, sizeof (unique_service),
DBUS_MPRIS_BUS_NAME"."DBUS_INSTANCE_ID_PREFIX"%"PRIu32,
(uint32_t)getpid() );
if( dbus_bus_request_name( p_conn, unique_service, bus_flags, NULL )
== DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER )
{
msg_Dbg( p_intf, "listening on dbus as: %s", unique_service );
var_SetString(vlc, "dbus-mpris-name", unique_service);
}
}
else
{
msg_Dbg( p_intf, "listening on dbus as: %s", DBUS_MPRIS_BUS_NAME );
var_SetString(vlc, "dbus-mpris-name", DBUS_MPRIS_BUS_NAME);
}
dbus_connection_flush( p_conn );
p_intf->p_sys = p_sys;
p_sys->p_conn = p_conn;
vlc_array_init( &p_sys->events );
vlc_array_init( &p_sys->timeouts );
vlc_array_init( &p_sys->watches );
vlc_mutex_init( &p_sys->lock );
p_sys->playlist = playlist = vlc_intf_GetMainPlaylist(p_intf);
vlc_player_t *player = vlc_playlist_GetPlayer(playlist);
vlc_playlist_Lock(playlist);
static struct vlc_playlist_callbacks const playlist_cbs =
{
.on_items_added = playlist_on_items_added,
.on_items_removed = playlist_on_items_removed,
.on_playback_repeat_changed = playlist_on_playback_repeat_changed,
.on_playback_order_changed = playlist_on_playback_order_changed,
.on_current_index_changed = playlist_on_current_index_changed,
};
p_sys->playlist_listener =
vlc_playlist_AddListener(playlist, &playlist_cbs, p_intf, false);
if (!p_sys->playlist_listener)
goto playlist_listener_failure;
static struct vlc_player_cbs const player_cbs =
{
.on_state_changed = player_on_state_changed,
.on_error_changed = player_on_error_changed,
.on_rate_changed = player_on_rate_changed,
.on_capabilities_changed = player_on_capabilities_changed,
.on_position_changed = player_on_position_changed,
.on_media_meta_changed = player_on_media_meta_changed,
};
p_sys->player_listener =
vlc_player_AddListener(player, &player_cbs, p_intf);
if (!p_sys->player_listener)
goto player_listener_failure;
static struct vlc_player_aout_cbs const player_aout_cbs =
{
.on_volume_changed = player_aout_on_volume_changed,
.on_mute_changed = player_aout_on_mute_changed
};
p_sys->player_aout_listener =
vlc_player_aout_AddListener(player, &player_aout_cbs, p_intf);
if (!p_sys->player_aout_listener)
goto player_aout_listener_failure;
static struct vlc_player_vout_cbs const player_vout_cbs =
{
.on_fullscreen_changed = player_vout_on_fullscreen_changed,
};
p_sys->player_vout_listener =
vlc_player_vout_AddListener(player, &player_vout_cbs, p_intf);
if (!p_sys->player_vout_listener)
goto player_vout_listener_failure;
vlc_playlist_Unlock(playlist);
if( !dbus_connection_set_timeout_functions( p_conn,
add_timeout,
remove_timeout,
toggle_timeout,
p_intf, NULL ) )
goto late_failure;
if( !dbus_connection_set_watch_functions( p_conn,
add_watch,
remove_watch,
watch_toggled,
p_intf, NULL ) )
goto late_failure;
if( vlc_clone( &p_sys->thread, Run, p_intf, VLC_THREAD_PRIORITY_LOW ) )
goto late_failure;
return VLC_SUCCESS;
late_failure:
vlc_playlist_Lock(playlist);
player_vout_listener_failure:
vlc_player_vout_RemoveListener(player, p_sys->player_vout_listener);
player_aout_listener_failure:
vlc_player_aout_RemoveListener(player, p_sys->player_aout_listener);
player_listener_failure:
vlc_player_RemoveListener(player, p_sys->player_listener);
playlist_listener_failure:
vlc_playlist_RemoveListener(playlist, p_sys->playlist_listener);
vlc_playlist_Unlock(playlist);
var_Destroy(vlc, "dbus-mpris-name");
dbus_connection_close( p_sys->p_conn );
dbus_connection_unref( p_conn );
dbus_connection_failure:
vlc_close( p_sys->p_pipe_fds[1] );
vlc_close( p_sys->p_pipe_fds[0] );
free( p_sys );
return VLC_EGENERIC;
}
static void Close ( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t*) p_this;
intf_sys_t *p_sys = p_intf->p_sys;
vlc_playlist_t *playlist = p_sys->playlist;
vlc_cancel( p_sys->thread );
vlc_join( p_sys->thread, NULL );
vlc_player_t *player = vlc_playlist_GetPlayer(playlist);
vlc_playlist_Lock(playlist);
vlc_player_vout_RemoveListener(player, p_sys->player_vout_listener);
vlc_player_aout_RemoveListener(player, p_sys->player_aout_listener);
vlc_player_RemoveListener(player, p_sys->player_listener);
vlc_playlist_RemoveListener(playlist, p_sys->playlist_listener);
vlc_playlist_Unlock(playlist);
dbus_connection_close( p_sys->p_conn );
dbus_connection_unref( p_sys->p_conn );
for( size_t i = 0; i < vlc_array_count( &p_sys->events ); i++ )
{
callback_info_t* info = vlc_array_item_at_index( &p_sys->events, i );
free( info );
}
vlc_array_clear( &p_sys->events );
vlc_array_clear( &p_sys->timeouts );
vlc_array_clear( &p_sys->watches );
vlc_close( p_sys->p_pipe_fds[1] );
vlc_close( p_sys->p_pipe_fds[0] );
free( p_sys );
}
static dbus_bool_t add_timeout(DBusTimeout *to, void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
vlc_tick_t *expiry = malloc(sizeof (*expiry));
if (unlikely(expiry == NULL))
return FALSE;
dbus_timeout_set_data(to, expiry, free);
vlc_mutex_lock(&sys->lock);
vlc_array_append_or_abort(&sys->timeouts, to);
vlc_mutex_unlock(&sys->lock);
return TRUE;
}
static void remove_timeout(DBusTimeout *to, void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
size_t idx;
vlc_mutex_lock(&sys->lock);
idx = vlc_array_index_of_item(&sys->timeouts, to);
vlc_array_remove(&sys->timeouts, idx);
vlc_mutex_unlock(&sys->lock);
}
static void toggle_timeout(DBusTimeout *to, void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
vlc_tick_t *expiry = dbus_timeout_get_data(to);
vlc_mutex_lock(&sys->lock);
if (dbus_timeout_get_enabled(to))
*expiry = vlc_tick_now() + UINT64_C(1000) * dbus_timeout_get_interval(to);
vlc_mutex_unlock(&sys->lock);
wakeup_main_loop(intf);
}
static int next_timeout(intf_thread_t *intf)
{
intf_sys_t *sys = intf->p_sys;
vlc_tick_t next_timeout = INT64_MAX;
unsigned count = vlc_array_count(&sys->timeouts);
for (unsigned i = 0; i < count; i++)
{
DBusTimeout *to = vlc_array_item_at_index(&sys->timeouts, i);
if (!dbus_timeout_get_enabled(to))
continue;
vlc_tick_t *expiry = dbus_timeout_get_data(to);
if (next_timeout > *expiry)
next_timeout = *expiry;
}
if (next_timeout >= INT64_MAX)
return -1;
next_timeout /= 1000;
if (next_timeout > INT_MAX)
return INT_MAX;
return (int)next_timeout;
}
static void process_timeouts(intf_thread_t *intf)
{
intf_sys_t *sys = intf->p_sys;
for (size_t i = 0; i < vlc_array_count(&sys->timeouts); i++)
{
DBusTimeout *to = vlc_array_item_at_index(&sys->timeouts, i);
if (!dbus_timeout_get_enabled(to))
continue;
vlc_tick_t *expiry = dbus_timeout_get_data(to);
if (*expiry > vlc_tick_now())
continue;
expiry += UINT64_C(1000) * dbus_timeout_get_interval(to);
vlc_mutex_unlock(&sys->lock);
dbus_timeout_handle(to);
vlc_mutex_lock(&sys->lock);
i = -1; 
}
}
static dbus_bool_t add_watch( DBusWatch *p_watch, void *p_data )
{
intf_thread_t *p_intf = (intf_thread_t*) p_data;
intf_sys_t *p_sys = (intf_sys_t*) p_intf->p_sys;
vlc_mutex_lock( &p_sys->lock );
vlc_array_append_or_abort( &p_sys->watches, p_watch );
vlc_mutex_unlock( &p_sys->lock );
return TRUE;
}
static void remove_watch( DBusWatch *p_watch, void *p_data )
{
intf_thread_t *p_intf = (intf_thread_t*) p_data;
intf_sys_t *p_sys = (intf_sys_t*) p_intf->p_sys;
size_t idx;
vlc_mutex_lock( &p_sys->lock );
idx = vlc_array_index_of_item( &p_sys->watches, p_watch );
vlc_array_remove( &p_sys->watches, idx );
vlc_mutex_unlock( &p_sys->lock );
}
static void watch_toggled( DBusWatch *p_watch, void *p_data )
{
intf_thread_t *p_intf = (intf_thread_t*) p_data;
if( dbus_watch_get_enabled( p_watch ) )
wakeup_main_loop( p_intf );
}
static int GetPollFds( intf_thread_t *p_intf, struct pollfd *p_fds )
{
intf_sys_t *p_sys = p_intf->p_sys;
size_t i_watches = vlc_array_count( &p_sys->watches );
int i_fds = 1;
p_fds[0].fd = p_sys->p_pipe_fds[PIPE_OUT];
p_fds[0].events = POLLIN | POLLPRI;
for( size_t i = 0; i < i_watches; i++ )
{
DBusWatch *p_watch = NULL;
p_watch = vlc_array_item_at_index( &p_sys->watches, i );
if( !dbus_watch_get_enabled( p_watch ) )
continue;
p_fds[i_fds].fd = dbus_watch_get_unix_fd( p_watch );
int i_flags = dbus_watch_get_flags( p_watch );
if( i_flags & DBUS_WATCH_READABLE )
p_fds[i_fds].events |= POLLIN | POLLPRI;
if( i_flags & DBUS_WATCH_WRITABLE )
p_fds[i_fds].events |= POLLOUT;
i_fds++;
}
return i_fds;
}
static void ProcessEvents( intf_thread_t *p_intf,
callback_info_t **p_events, int i_events )
{
bool b_can_play = p_intf->p_sys->b_can_play;
vlc_dictionary_t player_properties, tracklist_properties, root_properties;
vlc_dictionary_init( &player_properties, 0 );
vlc_dictionary_init( &tracklist_properties, 0 );
vlc_dictionary_init( &root_properties, 0 );
for( int i = 0; i < i_events; i++ )
{
switch( p_events[i]->signal )
{
case SIGNAL_ITEM_CURRENT:
TrackChange( p_intf );
if( !vlc_dictionary_has_key( &player_properties, "Rate" ) )
vlc_dictionary_insert( &player_properties, "Rate", NULL );
vlc_dictionary_insert( &player_properties, "Metadata", NULL );
break;
case SIGNAL_PLAYLIST_ITEM_APPEND:
case SIGNAL_PLAYLIST_ITEM_DELETED:
{
vlc_playlist_t *playlist = p_intf->p_sys->playlist;
vlc_playlist_Lock(playlist);
b_can_play = vlc_playlist_Count(playlist) > 0;
vlc_playlist_Unlock(playlist);
if( b_can_play != p_intf->p_sys->b_can_play )
{
p_intf->p_sys->b_can_play = b_can_play;
vlc_dictionary_insert( &player_properties, "CanPlay", NULL );
}
if( !vlc_dictionary_has_key( &tracklist_properties, "Tracks" ) )
vlc_dictionary_insert( &tracklist_properties, "Tracks", NULL );
break;
}
case SIGNAL_VOLUME_MUTED:
case SIGNAL_VOLUME_CHANGE:
vlc_dictionary_insert( &player_properties, "Volume", NULL );
break;
case SIGNAL_RANDOM:
vlc_dictionary_insert( &player_properties, "Shuffle", NULL );
break;
case SIGNAL_FULLSCREEN:
vlc_dictionary_insert( &root_properties, "Fullscreen", NULL );
break;
case SIGNAL_REPEAT:
case SIGNAL_LOOP:
vlc_dictionary_insert( &player_properties, "LoopStatus", NULL );
break;
case SIGNAL_STATE:
vlc_dictionary_insert( &player_properties, "PlaybackStatus", NULL );
break;
case SIGNAL_RATE:
vlc_dictionary_insert( &player_properties, "Rate", NULL );
break;
case SIGNAL_INPUT_METADATA:
{
vlc_player_t *player =
vlc_playlist_GetPlayer(p_intf->p_sys->playlist);
vlc_player_Lock(player);
input_item_t *p_item = vlc_player_GetCurrentMedia(player);
if( p_item )
vlc_dictionary_insert( &player_properties,
"Metadata", NULL );
vlc_player_Unlock(player);
break;
}
case SIGNAL_CAN_SEEK:
vlc_dictionary_insert( &player_properties, "CanSeek", NULL );
break;
case SIGNAL_CAN_PAUSE:
vlc_dictionary_insert( &player_properties, "CanPause", NULL );
break;
case SIGNAL_SEEK:
SeekedEmit( p_intf );
break;
default:
vlc_assert_unreachable();
}
free( p_events[i] );
}
if( !vlc_dictionary_is_empty( &player_properties ) )
PlayerPropertiesChangedEmit( p_intf, &player_properties );
if( !vlc_dictionary_is_empty( &tracklist_properties ) )
TrackListPropertiesChangedEmit( p_intf, &tracklist_properties );
if( !vlc_dictionary_is_empty( &root_properties ) )
RootPropertiesChangedEmit( p_intf, &root_properties );
vlc_dictionary_clear( &player_properties, NULL, NULL );
vlc_dictionary_clear( &tracklist_properties, NULL, NULL );
vlc_dictionary_clear( &root_properties, NULL, NULL );
}
static void ProcessWatches( intf_thread_t *p_intf,
DBusWatch **p_watches, int i_watches,
struct pollfd *p_fds, int i_fds )
{
VLC_UNUSED(p_intf);
for( int i = 0; i < i_watches; i++ )
{
DBusWatch *p_watch = p_watches[i];
if( !dbus_watch_get_enabled( p_watch ) )
continue;
for( int j = 0; j < i_fds; j++ )
{
if( p_fds[j].fd != dbus_watch_get_unix_fd( p_watch ) )
continue;
int i_flags = 0;
int i_revents = p_fds[j].revents;
if( i_revents & POLLIN )
i_flags |= DBUS_WATCH_READABLE;
if( i_revents & POLLOUT )
i_flags |= DBUS_WATCH_WRITABLE;
if( i_revents & POLLERR )
i_flags |= DBUS_WATCH_ERROR;
if( i_revents & POLLHUP )
i_flags |= DBUS_WATCH_HANGUP;
if( i_flags )
dbus_watch_handle( p_watch, i_flags );
}
}
}
static void DispatchDBusMessages( intf_thread_t *p_intf )
{
DBusDispatchStatus status;
intf_sys_t *p_sys = p_intf->p_sys;
status = dbus_connection_get_dispatch_status( p_sys->p_conn );
while( status != DBUS_DISPATCH_COMPLETE )
{
dbus_connection_dispatch( p_sys->p_conn );
status = dbus_connection_get_dispatch_status( p_sys->p_conn );
}
if( dbus_connection_has_messages_to_send( p_sys->p_conn ) )
dbus_connection_flush( p_sys->p_conn );
}
static DBusHandlerResult
MPRISEntryPoint ( DBusConnection *p_conn, DBusMessage *p_from, void *p_this )
{
const char *psz_target_interface;
const char *psz_interface = dbus_message_get_interface( p_from );
const char *psz_method = dbus_message_get_member( p_from );
DBusError error;
if( psz_interface && strcmp( psz_interface, DBUS_INTERFACE_PROPERTIES ) )
psz_target_interface = psz_interface;
else
{
dbus_error_init( &error );
dbus_message_get_args( p_from, &error,
DBUS_TYPE_STRING, &psz_target_interface,
DBUS_TYPE_INVALID );
if( dbus_error_is_set( &error ) )
{
msg_Err( (vlc_object_t*) p_this, "D-Bus error on %s.%s: %s",
psz_interface, psz_method,
error.message );
dbus_error_free( &error );
return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
}
if( !strcmp( psz_target_interface, DBUS_INTERFACE_INTROSPECTABLE ) )
return handle_introspect( p_conn, p_from, p_this );
if( !strcmp( psz_target_interface, DBUS_MPRIS_ROOT_INTERFACE ) )
return handle_root( p_conn, p_from, p_this );
if( !strcmp( psz_target_interface, DBUS_MPRIS_PLAYER_INTERFACE ) )
return handle_player( p_conn, p_from, p_this );
if( !strcmp( psz_target_interface, DBUS_MPRIS_TRACKLIST_INTERFACE ) )
return handle_tracklist( p_conn, p_from, p_this );
return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
static void *Run( void *data )
{
intf_thread_t *p_intf = data;
intf_sys_t *p_sys = p_intf->p_sys;
int canc = vlc_savecancel();
vlc_tick_t events_last_date = VLC_TICK_INVALID;
int events_poll_timeout = -1;
for( ;; )
{
vlc_mutex_lock( &p_sys->lock );
size_t i_watches = vlc_array_count( &p_sys->watches );
struct pollfd fds[i_watches];
memset(fds, 0, sizeof fds);
int i_fds = GetPollFds( p_intf, fds );
int timeout = next_timeout(p_intf);
vlc_mutex_unlock( &p_sys->lock );
vlc_restorecancel( canc );
if( timeout == -1 )
timeout = events_poll_timeout;
while (poll(fds, i_fds, timeout) == -1)
{
if (errno != EINTR)
goto error;
}
canc = vlc_savecancel();
if (fds[0].revents & POLLIN)
{
char buf;
(void)read( fds[0].fd, &buf, 1 );
}
vlc_mutex_lock( &p_intf->p_sys->lock );
process_timeouts(p_intf);
i_watches = vlc_array_count( &p_sys->watches );
DBusWatch *p_watches[i_watches ? i_watches : 1];
for( size_t i = 0; i < i_watches; i++ )
{
p_watches[i] = vlc_array_item_at_index( &p_sys->watches, i );
}
size_t i_events = vlc_array_count( &p_sys->events );
callback_info_t** pp_info = NULL;
if( i_events > 0 )
{
vlc_tick_t now = vlc_tick_now();
if( events_last_date == VLC_TICK_INVALID
|| now - events_last_date > EVENTS_DELAY )
{
events_last_date = now;
events_poll_timeout = -1;
pp_info = vlc_alloc( i_events, sizeof(*pp_info) );
if( pp_info )
{
for( size_t i = 0; i < i_events; i++ )
pp_info[i] = vlc_array_item_at_index( &p_sys->events, i );
vlc_array_clear( &p_sys->events );
}
}
else if( events_poll_timeout == -1 )
{
events_poll_timeout = ( EVENTS_DELAY - ( now - events_last_date ) ) / 1000;
}
}
else 
events_poll_timeout = -1;
vlc_mutex_unlock( &p_intf->p_sys->lock );
if( pp_info )
{
ProcessEvents( p_intf, pp_info, i_events );
free( pp_info );
}
ProcessWatches( p_intf, p_watches, i_watches, fds, i_fds );
DispatchDBusMessages( p_intf );
}
error:
vlc_restorecancel(canc);
return NULL;
}
static void wakeup_main_loop( void *p_data )
{
intf_thread_t *p_intf = (intf_thread_t*) p_data;
if( !write( p_intf->p_sys->p_pipe_fds[PIPE_IN], "\0", 1 ) )
msg_Err( p_intf, "Could not wake up the main loop: %s",
vlc_strerror_c(errno) );
}
static bool add_event_locked( intf_thread_t *p_intf, const callback_info_t *p_info )
{
if( !p_info->signal )
return false;
for( size_t i = 0; i < vlc_array_count( &p_intf->p_sys->events ); ++ i )
{
callback_info_t *oldinfo =
vlc_array_item_at_index( &p_intf->p_sys->events, i );
if( p_info->signal == oldinfo->signal )
return false;
}
callback_info_t *p_dup = malloc( sizeof( *p_dup ) );
if( unlikely(p_dup == NULL) )
return false;
*p_dup = *p_info;
vlc_array_append( &p_intf->p_sys->events, p_dup );
return true;
}
static bool add_event_signal( intf_thread_t *p_intf, const callback_info_t *p_info )
{
intf_sys_t *p_sys = p_intf->p_sys;
vlc_mutex_lock(&p_sys->lock);
bool added = add_event_locked(p_intf, p_info);
vlc_mutex_unlock(&p_sys->lock);
if( added )
wakeup_main_loop( p_intf );
return added;
}
static void
playlist_on_items_added(vlc_playlist_t *playlist, size_t index,
vlc_playlist_item_t *const items[], size_t count,
void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_PLAYLIST_ITEM_APPEND });
(void) playlist; (void) index; (void) items; (void) count;
}
static void
playlist_on_items_removed(vlc_playlist_t *playlist,
size_t index, size_t count, void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_PLAYLIST_ITEM_DELETED });
(void) playlist; (void) index; (void) count;
}
static void
playlist_on_playback_repeat_changed(vlc_playlist_t *playlist,
enum vlc_playlist_playback_repeat repeat,
void *data)
{
add_event_signal(data, &(callback_info_t){ .signal = SIGNAL_REPEAT });
(void) playlist; (void) repeat;
}
static void
playlist_on_playback_order_changed(vlc_playlist_t *playlist,
enum vlc_playlist_playback_order order,
void *data)
{
add_event_signal(data, &(callback_info_t){ .signal = SIGNAL_RANDOM });
(void) playlist; (void) order;
}
static void
playlist_on_current_index_changed(vlc_playlist_t *playlist,
ssize_t index, void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_ITEM_CURRENT });
(void) playlist; (void) index;
}
static void
player_on_state_changed(vlc_player_t *player, enum vlc_player_state state,
void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
dbus_int32_t playing_state;
switch (state)
{
case VLC_PLAYER_STATE_STARTED:
case VLC_PLAYER_STATE_PLAYING:
playing_state = PLAYBACK_STATE_PLAYING;
break;
case VLC_PLAYER_STATE_PAUSED:
playing_state = PLAYBACK_STATE_PAUSED;
break;
case VLC_PLAYER_STATE_STOPPED:
default:
playing_state = PLAYBACK_STATE_STOPPED;
break;
}
bool added = false;
vlc_mutex_lock(&sys->lock);
if (playing_state != sys->i_playing_state)
{
sys->i_playing_state = playing_state;
added = add_event_locked(intf,
&(callback_info_t) { .signal = SIGNAL_STATE });
}
vlc_mutex_unlock(&sys->lock);
if (added)
wakeup_main_loop(intf);
(void) player;
}
static void
player_on_error_changed(vlc_player_t *player, enum vlc_player_error error,
void *data)
{
if (error == VLC_PLAYER_ERROR_GENERIC)
player_on_state_changed(player, VLC_PLAYER_STATE_STOPPED, data);
}
static void
player_on_rate_changed(vlc_player_t *player, float new_rate, void *data)
{
add_event_signal(data, &(callback_info_t){ .signal = SIGNAL_RATE });
(void) player; (void) new_rate;
}
static void
player_on_capabilities_changed(vlc_player_t *player, int old_caps, int new_caps,
void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
vlc_mutex_lock(&sys->lock);
bool ok1 = add_event_locked(intf,
&(callback_info_t) { .signal = SIGNAL_CAN_SEEK });
bool ok2 = add_event_locked(intf,
&(callback_info_t) { .signal = SIGNAL_CAN_PAUSE });
vlc_mutex_unlock(&sys->lock);
if (ok1 || ok2)
wakeup_main_loop(intf);
(void) player; (void) old_caps; (void) new_caps;
}
static void
player_on_position_changed(vlc_player_t *player, vlc_tick_t time, float pos,
void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;
vlc_tick_t i_now = vlc_tick_now(), i_projected_pos, i_interval;
float f_current_rate;
if( !sys->i_last_input_pos_event ||
vlc_player_GetState(player) != VLC_PLAYER_STATE_PLAYING )
{
sys->i_last_input_pos_event = i_now;
sys->i_last_input_pos = pos;
return;
}
f_current_rate = vlc_player_GetRate(player);
i_interval = ( i_now - sys->i_last_input_pos_event );
i_projected_pos = sys->i_last_input_pos + ( i_interval * f_current_rate );
sys->i_last_input_pos_event = i_now;
sys->i_last_input_pos = pos;
if( llabs( pos - i_projected_pos ) < SEEK_THRESHOLD )
return;
add_event_signal(intf, &(callback_info_t){ .signal = SIGNAL_SEEK });
(void) time;
}
static void
player_on_media_meta_changed(vlc_player_t *player,
input_item_t *item, void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_INPUT_METADATA });
(void) player; (void) item;
}
static void
player_aout_on_volume_changed(audio_output_t *aout, float volume, void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_VOLUME_CHANGE });
(void) aout; (void) volume;
}
static void
player_aout_on_mute_changed(audio_output_t *aout, bool muted, void *data)
{
add_event_signal(data,
&(callback_info_t){ .signal = SIGNAL_VOLUME_MUTED });
(void) aout; (void) muted;
}
static void
player_vout_on_fullscreen_changed(vout_thread_t *vout, bool enabled,
void *data)
{
add_event_signal(data, &(callback_info_t){ .signal = SIGNAL_FULLSCREEN });
(void) vout; (void) enabled;
}
static int TrackChange( intf_thread_t *p_intf )
{
intf_sys_t *p_sys = p_intf->p_sys;
if( p_intf->p_sys->b_dead )
return VLC_SUCCESS;
p_sys->has_input = false;
p_sys->b_meta_read = false;
vlc_player_t *player = vlc_playlist_GetPlayer(p_sys->playlist);
vlc_player_Lock(player);
input_item_t *item = vlc_player_GetCurrentMedia(player);
vlc_player_Unlock(player);
if( !item )
return VLC_EGENERIC;
if( input_item_IsPreparsed( item ) )
p_sys->b_meta_read = true;
p_sys->has_input = true;
return VLC_SUCCESS;
}
int DemarshalSetPropertyValue( DBusMessage *p_msg, void *p_arg )
{
int i_type;
bool b_valid_input = FALSE;
DBusMessageIter in_args, variant;
dbus_message_iter_init( p_msg, &in_args );
do
{
i_type = dbus_message_iter_get_arg_type( &in_args );
if( DBUS_TYPE_VARIANT == i_type )
{
dbus_message_iter_recurse( &in_args, &variant );
dbus_message_iter_get_basic( &variant, p_arg );
b_valid_input = TRUE;
}
} while( dbus_message_iter_next( &in_args ) );
return b_valid_input ? VLC_SUCCESS : VLC_EGENERIC;
}
#define ADD_META( entry, type, data ) if( data ) { dbus_message_iter_open_container( &dict, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry ); dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &ppsz_meta_items[entry] ); dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, type##_AS_STRING, &variant ); dbus_message_iter_append_basic( &variant, type, & data ); dbus_message_iter_close_container( &dict_entry, &variant ); dbus_message_iter_close_container( &dict, &dict_entry ); }
#define ADD_VLC_META_STRING( entry, item ) { char * psz = input_item_Get##item( p_input );ADD_META( entry, DBUS_TYPE_STRING, psz ); free( psz ); }
#define ADD_META_SINGLETON_STRING_LIST( entry, item ) { char * psz = input_item_Get##item( p_input );if( psz ) { dbus_message_iter_open_container( &dict, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry ); dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &ppsz_meta_items[entry] ); dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "as", &variant ); dbus_message_iter_open_container( &variant, DBUS_TYPE_ARRAY, "s", &list ); dbus_message_iter_append_basic( &list, DBUS_TYPE_STRING, &psz ); dbus_message_iter_close_container( &variant, &list ); dbus_message_iter_close_container( &dict_entry, &variant ); dbus_message_iter_close_container( &dict, &dict_entry ); } free( psz ); }
int GetInputMeta(vlc_playlist_t *playlist, vlc_playlist_item_t *item, DBusMessageIter *args)
{
input_item_t *p_input = vlc_playlist_item_GetMedia(item);
DBusMessageIter dict, dict_entry, variant, list;
dbus_int64_t i_mtime = input_item_GetDuration( p_input );
dbus_uint32_t i_time = i_mtime / 1000000;
dbus_int64_t i_length = i_mtime / 1000;
char *psz_trackid;
if (asprintf(&psz_trackid, MPRIS_TRACKID_FORMAT,
vlc_playlist_IndexOf(playlist, item)) == -1)
return VLC_ENOMEM;
const char* ppsz_meta_items[] =
{
"mpris:trackid", "xesam:url", "xesam:title", "xesam:artist",
"xesam:album", "xesam:tracknumber", "vlc:time", "mpris:length",
"xesam:genre", "xesam:userRating", "xesam:contentCreated",
"mpris:artUrl", "mb:trackId", "vlc:audio-bitrate",
"vlc:audio-samplerate", "vlc:video-bitrate", "vlc:audio-codec",
"vlc:copyright", "xesam:comment", "vlc:encodedby", "language",
"vlc:length", "vlc:nowplaying", "vlc:publisher", "vlc:setting",
"status", "vlc:url", "vlc:video-codec"
};
dbus_message_iter_open_container( args, DBUS_TYPE_ARRAY, "{sv}", &dict );
ADD_META( 0, DBUS_TYPE_OBJECT_PATH, psz_trackid );
ADD_VLC_META_STRING( 1, URI );
ADD_VLC_META_STRING( 2, Title );
ADD_META_SINGLETON_STRING_LIST( 3, Artist );
ADD_VLC_META_STRING( 4, Album );
ADD_VLC_META_STRING( 5, TrackNum );
ADD_META( 6, DBUS_TYPE_UINT32, i_time );
ADD_META( 7, DBUS_TYPE_INT64, i_mtime );
ADD_META_SINGLETON_STRING_LIST( 8, Genre );
ADD_VLC_META_STRING( 10, Date ); 
ADD_VLC_META_STRING( 11, ArtURL );
ADD_VLC_META_STRING( 12, TrackID );
ADD_VLC_META_STRING( 17, Copyright );
ADD_META_SINGLETON_STRING_LIST( 18, Description );
ADD_VLC_META_STRING( 19, EncodedBy );
ADD_VLC_META_STRING( 20, Language );
ADD_META( 21, DBUS_TYPE_INT64, i_length );
ADD_VLC_META_STRING( 22, NowPlaying );
ADD_VLC_META_STRING( 23, Publisher );
ADD_VLC_META_STRING( 24, Setting );
ADD_VLC_META_STRING( 25, URL );
free( psz_trackid );
vlc_mutex_lock( &p_input->lock );
if( p_input->p_meta )
{
int i_status = vlc_meta_GetStatus( p_input->p_meta );
ADD_META( 23, DBUS_TYPE_INT32, i_status );
}
vlc_mutex_unlock( &p_input->lock );
dbus_message_iter_close_container( args, &dict );
return VLC_SUCCESS;
}
int AddProperty( intf_thread_t *p_intf,
DBusMessageIter *p_container,
const char* psz_property_name,
const char* psz_signature,
int (*pf_marshaller) (intf_thread_t*, DBusMessageIter*) )
{
DBusMessageIter entry, v;
if( !dbus_message_iter_open_container( p_container,
DBUS_TYPE_DICT_ENTRY, NULL,
&entry ) )
return VLC_ENOMEM;
if( !dbus_message_iter_append_basic( &entry,
DBUS_TYPE_STRING,
&psz_property_name ) )
return VLC_ENOMEM;
if( !dbus_message_iter_open_container( &entry,
DBUS_TYPE_VARIANT, psz_signature,
&v ) )
return VLC_ENOMEM;
if( VLC_SUCCESS != pf_marshaller( p_intf, &v ) )
return VLC_ENOMEM;
if( !dbus_message_iter_close_container( &entry, &v) )
return VLC_ENOMEM;
if( !dbus_message_iter_close_container( p_container, &entry ) )
return VLC_ENOMEM;
return VLC_SUCCESS;
}
#undef ADD_META
#undef ADD_VLC_META_STRING
