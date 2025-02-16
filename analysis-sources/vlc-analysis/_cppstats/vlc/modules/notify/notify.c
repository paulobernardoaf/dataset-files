#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_player.h>
#include <vlc_playlist.h>
#include <vlc_url.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libnotify/notify.h>
#if !defined(NOTIFY_CHECK_VERSION)
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
#define APPLICATION_NAME "VLC media player"
#define TIMEOUT_TEXT N_("Timeout (ms)")
#define TIMEOUT_LONGTEXT N_("How long the notification will be displayed.")
vlc_module_begin ()
set_category( CAT_INTERFACE )
set_subcategory( SUBCAT_INTERFACE_CONTROL )
set_shortname( N_( "Notify" ) )
set_description( N_("LibNotify Notification Plugin") )
add_integer( "notify-timeout", 4000,
TIMEOUT_TEXT, TIMEOUT_LONGTEXT, true )
set_capability( "interface", 0 )
set_callbacks( Open, Close )
vlc_module_end ()
static void on_current_media_changed(vlc_player_t *player,
input_item_t *new_media, void *data);
static int Notify( vlc_object_t *, const char *, GdkPixbuf *, intf_thread_t * );
#define MAX_LENGTH 256
struct intf_sys_t
{
NotifyNotification *notification;
vlc_mutex_t lock;
bool b_has_actions;
vlc_playlist_t *playlist;
struct vlc_player_listener_id *player_listener;
};
static void foreach_g_free(void *data, void *userdata)
{
g_free(data);
VLC_UNUSED(userdata);
}
static int Open( vlc_object_t *p_this )
{
intf_thread_t *p_intf = (intf_thread_t *)p_this;
intf_sys_t *p_sys = malloc( sizeof( *p_sys ) );
if( !p_sys )
return VLC_ENOMEM;
p_sys->playlist = vlc_intf_GetMainPlaylist(p_intf);
vlc_player_t *player = vlc_playlist_GetPlayer(p_sys->playlist);
static const struct vlc_player_cbs player_cbs =
{
.on_current_media_changed = on_current_media_changed
};
vlc_player_Lock(player);
p_sys->player_listener = vlc_player_AddListener(player, &player_cbs, p_intf);
vlc_player_Unlock(player);
if (!p_sys->player_listener)
{
free(p_sys);
return VLC_EGENERIC;
}
if( !notify_init( APPLICATION_NAME ) )
{
vlc_player_Lock(player);
vlc_player_RemoveListener(player, p_sys->player_listener);
vlc_player_Unlock(player);
free( p_sys );
msg_Err( p_intf, "can't find notification daemon" );
return VLC_EGENERIC;
}
p_intf->p_sys = p_sys;
vlc_mutex_init( &p_sys->lock );
p_sys->notification = NULL;
p_sys->b_has_actions = false;
GList *p_caps = notify_get_server_caps ();
if( p_caps )
{
for( GList *c = p_caps; c != NULL; c = c->next )
{
if( !strcmp( (char*)c->data, "actions" ) )
{
p_sys->b_has_actions = true;
break;
}
}
g_list_foreach( p_caps, foreach_g_free, NULL );
g_list_free( p_caps );
}
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
intf_thread_t *p_intf = ( intf_thread_t* ) p_this;
intf_sys_t *p_sys = p_intf->p_sys;
vlc_player_t *player = vlc_playlist_GetPlayer(p_sys->playlist);
vlc_player_Lock(player);
vlc_player_RemoveListener(player, p_sys->player_listener);
vlc_player_Unlock(player);
if( p_sys->notification )
{
GError *p_error = NULL;
notify_notification_close( p_sys->notification, &p_error );
g_object_unref( p_sys->notification );
}
free( p_sys );
notify_uninit();
}
static void on_current_media_changed(vlc_player_t *player,
input_item_t *p_input_item, void *data)
{
(void) player;
intf_thread_t *p_intf = data;
intf_sys_t *p_sys = p_intf->p_sys;
char psz_tmp[MAX_LENGTH];
char psz_notify[MAX_LENGTH];
char *psz_title;
char *psz_artist;
char *psz_album;
char *psz_arturl;
if (!p_input_item)
return;
if(p_input_item->i_type == ITEM_TYPE_DIRECTORY || p_input_item->i_type == ITEM_TYPE_PLAYLIST
|| p_input_item->i_type == ITEM_TYPE_NODE || p_input_item->i_type== ITEM_TYPE_UNKNOWN
|| p_input_item->i_type == ITEM_TYPE_CARD){
return;
}
psz_title = input_item_GetTitleFbName( p_input_item );
if( EMPTY_STR( psz_title ) )
{
free( psz_title );
return;
}
psz_artist = input_item_GetArtist( p_input_item );
psz_album = input_item_GetAlbum( p_input_item );
if( !EMPTY_STR( psz_artist ) )
{
if( !EMPTY_STR( psz_album ) )
snprintf( psz_tmp, MAX_LENGTH, "<b>%s</b>\n%s\n[%s]",
psz_title, psz_artist, psz_album );
else
snprintf( psz_tmp, MAX_LENGTH, "<b>%s</b>\n%s",
psz_title, psz_artist );
}
else
snprintf( psz_tmp, MAX_LENGTH, "<b>%s</b>", psz_title );
free( psz_title );
free( psz_artist );
free( psz_album );
GdkPixbuf *pix = NULL;
psz_arturl = input_item_GetArtURL( p_input_item );
if( psz_arturl )
{
char *psz = vlc_uri2path( psz_arturl );
free( psz_arturl );
psz_arturl = psz;
}
if( psz_arturl )
{ 
GError *p_error = NULL;
pix = gdk_pixbuf_new_from_file_at_scale( psz_arturl,
72, 72, TRUE, &p_error );
}
else 
{
GtkIconTheme* p_theme = gtk_icon_theme_get_default();
pix = gtk_icon_theme_load_icon( p_theme, "vlc", 72, 0, NULL);
if( !pix )
{
GError *p_error = NULL;
char *psz_pixbuf = config_GetSysPath(VLC_SYSDATA_DIR,
"icons/hicolor/48x48/"PACKAGE_NAME".png");
if (psz_pixbuf != NULL)
{
pix = gdk_pixbuf_new_from_file( psz_pixbuf, &p_error );
free( psz_pixbuf );
}
}
}
free( psz_arturl );
const int i_len = strlen( psz_tmp );
int i_notify = 0;
for( int i = 0; i < i_len && i_notify < ( MAX_LENGTH - 5 ); i++ )
{ 
if( psz_tmp[i] != '&' )
{
psz_notify[i_notify] = psz_tmp[i];
}
else
{
strcpy( &psz_notify[i_notify], "&amp;" );
i_notify += 4;
}
i_notify++;
}
psz_notify[i_notify] = '\0';
vlc_mutex_lock( &p_sys->lock );
Notify( VLC_OBJECT(p_intf), psz_notify, pix, p_intf );
vlc_mutex_unlock( &p_sys->lock );
}
static void Next( NotifyNotification *notification, gchar *psz, gpointer p )
{
intf_thread_t *p_intf = (intf_thread_t *)p;
intf_sys_t *p_sys = p_intf->p_sys;
VLC_UNUSED(psz);
notify_notification_close( notification, NULL );
vlc_playlist_Lock( p_sys->playlist );
vlc_playlist_Next( p_sys->playlist );
vlc_playlist_Unlock( p_sys->playlist );
}
static void Prev( NotifyNotification *notification, gchar *psz, gpointer p )
{
intf_thread_t *p_intf = (intf_thread_t *)p;
intf_sys_t *p_sys = p_intf->p_sys;
VLC_UNUSED(psz);
notify_notification_close( notification, NULL );
vlc_playlist_Lock( p_sys->playlist );
vlc_playlist_Prev( p_sys->playlist );
vlc_playlist_Unlock( p_sys->playlist );
}
static int Notify( vlc_object_t *p_this, const char *psz_temp, GdkPixbuf *pix,
intf_thread_t *p_intf )
{
intf_sys_t *p_sys = p_intf->p_sys;
NotifyNotification * notification;
if( p_sys->notification )
{
GError *p_error = NULL;
notify_notification_close( p_sys->notification, &p_error );
g_object_unref( p_sys->notification );
}
notification = notify_notification_new( _("Now Playing"),
psz_temp, NULL
#if NOTIFY_CHECK_VERSION (0, 7, 0)
);
#else
, NULL );
#endif
notify_notification_set_timeout( notification,
var_InheritInteger(p_this, "notify-timeout") );
notify_notification_set_urgency( notification, NOTIFY_URGENCY_LOW );
if( pix )
{
notify_notification_set_icon_from_pixbuf( notification, pix );
g_object_unref( pix );
}
if( p_sys->b_has_actions )
{
notify_notification_add_action( notification, "previous", _("Previous"), Prev,
(gpointer*)p_intf, NULL );
notify_notification_add_action( notification, "next", _("Next"), Next,
(gpointer*)p_intf, NULL );
}
notify_notification_show( notification, NULL);
p_sys->notification = notification;
return VLC_SUCCESS;
}
