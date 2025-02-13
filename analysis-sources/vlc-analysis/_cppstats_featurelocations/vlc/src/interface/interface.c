

































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <unistd.h>

#include <vlc_common.h>
#include <vlc_modules.h>
#include <vlc_interface.h>
#include <vlc_playlist.h>
#include "libvlc.h"
#include "../lib/libvlc_internal.h"
#include "player/player.h"

static int AddIntfCallback( vlc_object_t *, char const *,
vlc_value_t , vlc_value_t , void * );

static void
PlaylistConfigureFromVariables(vlc_playlist_t *playlist, vlc_object_t *obj)
{
enum vlc_playlist_playback_order order;
if (var_InheritBool(obj, "random"))
order = VLC_PLAYLIST_PLAYBACK_ORDER_RANDOM;
else
order = VLC_PLAYLIST_PLAYBACK_ORDER_NORMAL;


enum vlc_playlist_playback_repeat repeat;
if (var_InheritBool(obj, "repeat"))
repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_CURRENT;
else if (var_InheritBool(obj, "loop"))
repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_ALL;
else
repeat = VLC_PLAYLIST_PLAYBACK_REPEAT_NONE;

enum vlc_player_media_stopped_action media_stopped_action;
if (var_InheritBool(obj, "play-and-exit"))
media_stopped_action = VLC_PLAYER_MEDIA_STOPPED_EXIT;
else if (var_InheritBool(obj, "play-and-stop"))
media_stopped_action = VLC_PLAYER_MEDIA_STOPPED_STOP;
else if (var_InheritBool(obj, "play-and-pause"))
media_stopped_action = VLC_PLAYER_MEDIA_STOPPED_PAUSE;
else
media_stopped_action = VLC_PLAYER_MEDIA_STOPPED_CONTINUE;

bool start_paused = var_InheritBool(obj, "start-paused");
bool playlist_cork = var_InheritBool(obj, "playlist-cork");

vlc_playlist_Lock(playlist);
vlc_playlist_SetPlaybackOrder(playlist, order);
vlc_playlist_SetPlaybackRepeat(playlist, repeat);

vlc_player_t *player = vlc_playlist_GetPlayer(playlist);



vlc_player_SetMediaStoppedAction(player, media_stopped_action);
vlc_player_SetStartPaused(player, start_paused);
vlc_player_SetPauseOnCork(player, playlist_cork);

vlc_playlist_Unlock(playlist);
}

static vlc_playlist_t *
libvlc_GetMainPlaylist(libvlc_int_t *libvlc)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);

vlc_mutex_lock(&priv->lock);
vlc_playlist_t *playlist = priv->main_playlist;
if (priv->main_playlist == NULL)
{
playlist = priv->main_playlist = vlc_playlist_New(VLC_OBJECT(libvlc));
if (playlist)
PlaylistConfigureFromVariables(playlist, VLC_OBJECT(libvlc));
}
vlc_mutex_unlock(&priv->lock);

return playlist;
}

vlc_playlist_t *
vlc_intf_GetMainPlaylist(intf_thread_t *intf)
{
vlc_playlist_t *pl = libvlc_GetMainPlaylist(vlc_object_instance(intf));
assert(pl);
return pl;
}








int intf_Create( libvlc_int_t *libvlc, const char *chain )
{
assert( libvlc );
libvlc_priv_t *priv = libvlc_priv(libvlc);


if (libvlc_GetMainPlaylist(libvlc) == NULL)
return VLC_ENOMEM;


intf_thread_t *p_intf = vlc_custom_create( libvlc, sizeof( *p_intf ),
"interface" );
if( unlikely(p_intf == NULL) )
return VLC_ENOMEM;


vlc_value_t val;
var_Create( p_intf, "intf-add", VLC_VAR_STRING | VLC_VAR_ISCOMMAND );
var_Change( p_intf, "intf-add", VLC_VAR_SETTEXT, _("Add Interface") );
#if !defined(_WIN32) && defined(HAVE_ISATTY)
if( isatty( 0 ) )
#endif
{
val.psz_string = (char *)"rc,none";
var_Change( p_intf, "intf-add", VLC_VAR_ADDCHOICE, val, _("Console") );
}
val.psz_string = (char *)"telnet,none";
var_Change( p_intf, "intf-add", VLC_VAR_ADDCHOICE, val, _("Telnet") );
val.psz_string = (char *)"http,none";
var_Change( p_intf, "intf-add", VLC_VAR_ADDCHOICE, val, _("Web") );
val.psz_string = (char *)"gestures,none";
var_Change( p_intf, "intf-add", VLC_VAR_ADDCHOICE, val,
_("Mouse Gestures") );

var_AddCallback( p_intf, "intf-add", AddIntfCallback, NULL );


char *module;

p_intf->p_cfg = NULL;
free( config_ChainCreate( &module, &p_intf->p_cfg, chain ) );
p_intf->p_module = module_need( p_intf, "interface", module, true );
free(module);
if( p_intf->p_module == NULL )
{
msg_Err( p_intf, "no suitable interface module" );
goto error;
}

vlc_mutex_lock(&priv->lock);
p_intf->p_next = priv->interfaces;
priv->interfaces = p_intf;
vlc_mutex_unlock(&priv->lock);

return VLC_SUCCESS;

error:
if( p_intf->p_module )
module_unneed( p_intf, p_intf->p_module );
config_ChainDestroy( p_intf->p_cfg );
vlc_object_delete(p_intf);
return VLC_EGENERIC;
}








int intf_InsertItem(libvlc_int_t *libvlc, const char *mrl, unsigned optc,
const char *const *optv, unsigned flags)
{
input_item_t *item = input_item_New(mrl, NULL);

if (unlikely(item == NULL))
return -1;

int ret = -1;

if (input_item_AddOptions(item, optc, optv, flags) == VLC_SUCCESS)
{
vlc_playlist_t *playlist = libvlc_GetMainPlaylist(libvlc);
if (playlist)
{
vlc_playlist_Lock(playlist);
ret = vlc_playlist_InsertOne(playlist, 0, item);
vlc_playlist_Unlock(playlist);
}
}
input_item_Release(item);
return ret;
}

void libvlc_InternalPlay(libvlc_int_t *libvlc)
{
if (!var_InheritBool(VLC_OBJECT(libvlc), "playlist-autostart"))
return;
vlc_playlist_t *playlist = libvlc_GetMainPlaylist(libvlc);
if (!playlist)
return;
vlc_playlist_Lock(playlist);
if (vlc_playlist_Count(playlist) > 0)
{
if (vlc_playlist_GetCurrentIndex(playlist) < 0)
vlc_playlist_GoTo(playlist, 0);
vlc_playlist_Start(playlist);
}
vlc_playlist_Unlock(playlist);
}




int libvlc_InternalAddIntf(libvlc_int_t *libvlc, const char *name)
{
int ret;

if (name != NULL)
ret = intf_Create(libvlc, name);
else
{ 
char *intf = var_InheritString(libvlc, "intf");
if (intf == NULL) 
msg_Info(libvlc, _("Running vlc with the default interface. "
"Use 'cvlc' to use vlc without interface."));

ret = intf_Create(libvlc, intf);
free(intf);
name = "default";
}
if (ret != VLC_SUCCESS)
msg_Err(libvlc, "interface \"%s\" initialization failed", name);
return ret;
}






void intf_DestroyAll(libvlc_int_t *libvlc)
{
libvlc_priv_t *priv = libvlc_priv(libvlc);

vlc_mutex_lock(&priv->lock);
intf_thread_t *intf, **pp = &priv->interfaces;

while ((intf = *pp) != NULL)
{
*pp = intf->p_next;
vlc_mutex_unlock(&priv->lock);

module_unneed(intf, intf->p_module);
config_ChainDestroy(intf->p_cfg);
var_DelCallback(intf, "intf-add", AddIntfCallback, NULL);
vlc_object_delete(intf);

vlc_mutex_lock(&priv->lock);
}
vlc_mutex_unlock(&priv->lock);
}



static int AddIntfCallback( vlc_object_t *obj, char const *var,
vlc_value_t old, vlc_value_t cur, void *data )
{
int ret = intf_Create( vlc_object_instance(obj), cur.psz_string );
if( ret )
msg_Err( obj, "interface \"%s\" initialization failed",
cur.psz_string );

(void) var; (void) old; (void) data;
return ret;
}
