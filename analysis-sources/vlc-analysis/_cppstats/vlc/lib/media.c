#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <errno.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_picture.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_list.h> 
#include <vlc/libvlc_events.h>
#include <vlc_common.h>
#include <vlc_meta.h>
#include <vlc_url.h>
#include <vlc_thumbnailer.h>
#include "../src/libvlc.h"
#include "libvlc_internal.h"
#include "media_internal.h"
#include "media_list_internal.h"
#include "picture_internal.h"
static const vlc_meta_type_t libvlc_to_vlc_meta[] =
{
[libvlc_meta_Title] = vlc_meta_Title,
[libvlc_meta_Artist] = vlc_meta_Artist,
[libvlc_meta_Genre] = vlc_meta_Genre,
[libvlc_meta_Copyright] = vlc_meta_Copyright,
[libvlc_meta_Album] = vlc_meta_Album,
[libvlc_meta_TrackNumber] = vlc_meta_TrackNumber,
[libvlc_meta_Description] = vlc_meta_Description,
[libvlc_meta_Rating] = vlc_meta_Rating,
[libvlc_meta_Date] = vlc_meta_Date,
[libvlc_meta_Setting] = vlc_meta_Setting,
[libvlc_meta_URL] = vlc_meta_URL,
[libvlc_meta_Language] = vlc_meta_Language,
[libvlc_meta_NowPlaying] = vlc_meta_NowPlaying,
[libvlc_meta_Publisher] = vlc_meta_Publisher,
[libvlc_meta_EncodedBy] = vlc_meta_EncodedBy,
[libvlc_meta_ArtworkURL] = vlc_meta_ArtworkURL,
[libvlc_meta_TrackID] = vlc_meta_TrackID,
[libvlc_meta_TrackTotal] = vlc_meta_TrackTotal,
[libvlc_meta_Director] = vlc_meta_Director,
[libvlc_meta_Season] = vlc_meta_Season,
[libvlc_meta_Episode] = vlc_meta_Episode,
[libvlc_meta_ShowName] = vlc_meta_ShowName,
[libvlc_meta_Actors] = vlc_meta_Actors,
[libvlc_meta_AlbumArtist] = vlc_meta_AlbumArtist,
[libvlc_meta_DiscNumber] = vlc_meta_DiscNumber,
[libvlc_meta_DiscTotal] = vlc_meta_DiscTotal
};
static const libvlc_meta_t vlc_to_libvlc_meta[] =
{
[vlc_meta_Title] = libvlc_meta_Title,
[vlc_meta_Artist] = libvlc_meta_Artist,
[vlc_meta_Genre] = libvlc_meta_Genre,
[vlc_meta_Copyright] = libvlc_meta_Copyright,
[vlc_meta_Album] = libvlc_meta_Album,
[vlc_meta_TrackNumber] = libvlc_meta_TrackNumber,
[vlc_meta_Description] = libvlc_meta_Description,
[vlc_meta_Rating] = libvlc_meta_Rating,
[vlc_meta_Date] = libvlc_meta_Date,
[vlc_meta_Setting] = libvlc_meta_Setting,
[vlc_meta_URL] = libvlc_meta_URL,
[vlc_meta_Language] = libvlc_meta_Language,
[vlc_meta_NowPlaying] = libvlc_meta_NowPlaying,
[vlc_meta_ESNowPlaying] = libvlc_meta_NowPlaying,
[vlc_meta_Publisher] = libvlc_meta_Publisher,
[vlc_meta_EncodedBy] = libvlc_meta_EncodedBy,
[vlc_meta_ArtworkURL] = libvlc_meta_ArtworkURL,
[vlc_meta_TrackID] = libvlc_meta_TrackID,
[vlc_meta_TrackTotal] = libvlc_meta_TrackTotal,
[vlc_meta_Director] = libvlc_meta_Director,
[vlc_meta_Season] = libvlc_meta_Season,
[vlc_meta_Episode] = libvlc_meta_Episode,
[vlc_meta_ShowName] = libvlc_meta_ShowName,
[vlc_meta_Actors] = libvlc_meta_Actors,
[vlc_meta_AlbumArtist] = libvlc_meta_AlbumArtist,
[vlc_meta_DiscNumber] = libvlc_meta_DiscNumber,
[vlc_meta_DiscTotal] = libvlc_meta_DiscTotal
};
static_assert(
ORIENT_TOP_LEFT == (int) libvlc_video_orient_top_left &&
ORIENT_TOP_RIGHT == (int) libvlc_video_orient_top_right &&
ORIENT_BOTTOM_LEFT == (int) libvlc_video_orient_bottom_left &&
ORIENT_BOTTOM_RIGHT == (int) libvlc_video_orient_bottom_right &&
ORIENT_LEFT_TOP == (int) libvlc_video_orient_left_top &&
ORIENT_LEFT_BOTTOM == (int) libvlc_video_orient_left_bottom &&
ORIENT_RIGHT_TOP == (int) libvlc_video_orient_right_top &&
ORIENT_RIGHT_BOTTOM == (int) libvlc_video_orient_right_bottom,
"Mismatch between libvlc_video_orient_t and video_orientation_t" );
static_assert(
PROJECTION_MODE_RECTANGULAR == (int) libvlc_video_projection_rectangular &&
PROJECTION_MODE_EQUIRECTANGULAR == (int) libvlc_video_projection_equirectangular &&
PROJECTION_MODE_CUBEMAP_LAYOUT_STANDARD == (int) libvlc_video_projection_cubemap_layout_standard,
"Mismatch between libvlc_video_projection_t and video_projection_mode_t" );
static_assert(
MULTIVIEW_2D == (int) libvlc_video_multiview_2d &&
MULTIVIEW_STEREO_SBS == (int) libvlc_video_multiview_stereo_sbs &&
MULTIVIEW_STEREO_TB == (int) libvlc_video_multiview_stereo_tb &&
MULTIVIEW_STEREO_ROW == (int) libvlc_video_multiview_stereo_row &&
MULTIVIEW_STEREO_COL == (int) libvlc_video_multiview_stereo_col &&
MULTIVIEW_STEREO_FRAME == (int) libvlc_video_multiview_stereo_frame &&
MULTIVIEW_STEREO_CHECKERBOARD == (int) libvlc_video_multiview_stereo_checkerboard,
"Mismatch between libvlc_video_multiview_t and video_multiview_mode_t");
static libvlc_media_list_t *media_get_subitems( libvlc_media_t * p_md,
bool b_create )
{
libvlc_media_list_t *p_subitems = NULL;
vlc_mutex_lock( &p_md->subitems_lock );
if( p_md->p_subitems == NULL && b_create )
{
p_md->p_subitems = libvlc_media_list_new( p_md->p_libvlc_instance );
if( p_md->p_subitems != NULL )
{
p_md->p_subitems->b_read_only = true;
p_md->p_subitems->p_internal_md = p_md;
}
}
p_subitems = p_md->p_subitems;
vlc_mutex_unlock( &p_md->subitems_lock );
return p_subitems;
}
static libvlc_media_t *input_item_add_subitem( libvlc_media_t *p_md,
input_item_t *item )
{
libvlc_media_t * p_md_child;
libvlc_media_list_t *p_subitems;
libvlc_event_t event;
p_md_child = libvlc_media_new_from_input_item( p_md->p_libvlc_instance,
item );
p_subitems = media_get_subitems( p_md, true );
if( p_subitems != NULL )
{
libvlc_media_list_lock( p_subitems );
libvlc_media_list_internal_add_media( p_subitems, p_md_child );
libvlc_media_list_unlock( p_subitems );
}
event.type = libvlc_MediaSubItemAdded;
event.u.media_subitem_added.new_child = p_md_child;
libvlc_event_send( &p_md->event_manager, &event );
return p_md_child;
}
static void input_item_add_subnode( libvlc_media_t *md,
input_item_node_t *node )
{
for( int i = 0; i < node->i_children; i++ )
{
input_item_node_t *child = node->pp_children[i];
libvlc_media_t *md_child = input_item_add_subitem( md, child->p_item );
if( md_child != NULL )
{
input_item_add_subnode( md_child, child );
libvlc_media_release( md_child );
}
}
}
static void input_item_subtree_added(input_item_t *item,
input_item_node_t *node,
void *user_data)
{
VLC_UNUSED(item);
libvlc_media_t * p_md = user_data;
libvlc_media_add_subtree(p_md, node);
}
void libvlc_media_add_subtree(libvlc_media_t *p_md, input_item_node_t *node)
{
input_item_add_subnode( p_md, node );
libvlc_event_t event;
event.type = libvlc_MediaSubItemTreeAdded;
event.u.media_subitemtree_added.item = p_md;
libvlc_event_send( &p_md->event_manager, &event );
}
static void input_item_meta_changed( const vlc_event_t *p_event,
void * user_data )
{
libvlc_media_t * p_md = user_data;
libvlc_event_t event;
event.type = libvlc_MediaMetaChanged;
event.u.media_meta_changed.meta_type =
vlc_to_libvlc_meta[p_event->u.input_item_meta_changed.meta_type];
libvlc_event_send( &p_md->event_manager, &event );
}
static void input_item_duration_changed( const vlc_event_t *p_event,
void * user_data )
{
libvlc_media_t * p_md = user_data;
libvlc_event_t event;
event.type = libvlc_MediaDurationChanged;
event.u.media_duration_changed.new_duration =
from_mtime(p_event->u.input_item_duration_changed.new_duration);
libvlc_event_send( &p_md->event_manager, &event );
}
static void send_parsed_changed( libvlc_media_t *p_md,
libvlc_media_parsed_status_t new_status )
{
libvlc_event_t event;
vlc_mutex_lock( &p_md->parsed_lock );
if( p_md->parsed_status == new_status )
{
vlc_mutex_unlock( &p_md->parsed_lock );
return;
}
if( !p_md->is_parsed )
{
p_md->is_parsed = true;
vlc_cond_broadcast( &p_md->parsed_cond );
}
p_md->parsed_status = new_status;
if( p_md->parsed_status != libvlc_media_parsed_status_done )
p_md->has_asked_preparse = false;
vlc_mutex_unlock( &p_md->parsed_lock );
if( new_status == libvlc_media_parsed_status_done )
{
libvlc_media_list_t *p_subitems = media_get_subitems( p_md, false );
if( p_subitems != NULL )
{
libvlc_media_list_lock( p_subitems );
libvlc_media_list_internal_end_reached( p_subitems );
libvlc_media_list_unlock( p_subitems );
}
}
event.type = libvlc_MediaParsedChanged;
event.u.media_parsed_changed.new_status = new_status;
libvlc_event_send( &p_md->event_manager, &event );
}
static void input_item_preparse_ended(input_item_t *item,
enum input_item_preparse_status status,
void *user_data)
{
VLC_UNUSED(item);
libvlc_media_t * p_md = user_data;
libvlc_media_parsed_status_t new_status;
switch( status )
{
case ITEM_PREPARSE_SKIPPED:
new_status = libvlc_media_parsed_status_skipped;
break;
case ITEM_PREPARSE_FAILED:
new_status = libvlc_media_parsed_status_failed;
break;
case ITEM_PREPARSE_TIMEOUT:
new_status = libvlc_media_parsed_status_timeout;
break;
case ITEM_PREPARSE_DONE:
new_status = libvlc_media_parsed_status_done;
break;
default:
return;
}
send_parsed_changed( p_md, new_status );
}
static void install_input_item_observer( libvlc_media_t *p_md )
{
vlc_event_attach( &p_md->p_input_item->event_manager,
vlc_InputItemMetaChanged,
input_item_meta_changed,
p_md );
vlc_event_attach( &p_md->p_input_item->event_manager,
vlc_InputItemDurationChanged,
input_item_duration_changed,
p_md );
}
static void uninstall_input_item_observer( libvlc_media_t *p_md )
{
vlc_event_detach( &p_md->p_input_item->event_manager,
vlc_InputItemMetaChanged,
input_item_meta_changed,
p_md );
vlc_event_detach( &p_md->p_input_item->event_manager,
vlc_InputItemDurationChanged,
input_item_duration_changed,
p_md );
}
libvlc_media_t * libvlc_media_new_from_input_item(
libvlc_instance_t *p_instance,
input_item_t *p_input_item )
{
libvlc_media_t * p_md;
if (!p_input_item)
{
libvlc_printerr( "No input item given" );
return NULL;
}
p_md = calloc( 1, sizeof(libvlc_media_t) );
if( !p_md )
{
libvlc_printerr( "Not enough memory" );
return NULL;
}
p_md->p_libvlc_instance = p_instance;
p_md->p_input_item = p_input_item;
p_md->i_refcount = 1;
vlc_cond_init(&p_md->parsed_cond);
vlc_mutex_init(&p_md->parsed_lock);
vlc_mutex_init(&p_md->subitems_lock);
p_md->state = libvlc_NothingSpecial;
p_md->p_subitems = NULL;
libvlc_event_manager_init( &p_md->event_manager, p_md );
input_item_Hold( p_md->p_input_item );
install_input_item_observer( p_md );
libvlc_retain( p_instance );
return p_md;
}
libvlc_media_t *libvlc_media_new_location( libvlc_instance_t *p_instance,
const char * psz_mrl )
{
input_item_t * p_input_item;
libvlc_media_t * p_md;
p_input_item = input_item_New( psz_mrl, NULL );
if (!p_input_item)
{
libvlc_printerr( "Not enough memory" );
return NULL;
}
p_md = libvlc_media_new_from_input_item( p_instance, p_input_item );
input_item_Release( p_input_item );
return p_md;
}
libvlc_media_t *libvlc_media_new_path( libvlc_instance_t *p_instance,
const char *path )
{
char *mrl = vlc_path2uri( path, NULL );
if( unlikely(mrl == NULL) )
{
libvlc_printerr( "%s", vlc_strerror_c(errno) );
return NULL;
}
libvlc_media_t *m = libvlc_media_new_location( p_instance, mrl );
free( mrl );
return m;
}
libvlc_media_t *libvlc_media_new_fd( libvlc_instance_t *p_instance, int fd )
{
char mrl[16];
snprintf( mrl, sizeof(mrl), "fd://%d", fd );
return libvlc_media_new_location( p_instance, mrl );
}
libvlc_media_t *libvlc_media_new_callbacks(libvlc_instance_t *p_instance,
libvlc_media_open_cb open_cb,
libvlc_media_read_cb read_cb,
libvlc_media_seek_cb seek_cb,
libvlc_media_close_cb close_cb,
void *opaque)
{
libvlc_media_t *m = libvlc_media_new_location(p_instance, "imem://");
if (unlikely(m == NULL))
return NULL;
assert(read_cb != NULL);
input_item_AddOpaque(m->p_input_item, "imem-data", opaque);
input_item_AddOpaque(m->p_input_item, "imem-open", open_cb);
input_item_AddOpaque(m->p_input_item, "imem-read", read_cb);
input_item_AddOpaque(m->p_input_item, "imem-seek", seek_cb);
input_item_AddOpaque(m->p_input_item, "imem-close", close_cb);
return m;
}
libvlc_media_t * libvlc_media_new_as_node( libvlc_instance_t *p_instance,
const char * psz_name )
{
input_item_t * p_input_item;
libvlc_media_t * p_md;
libvlc_media_list_t * p_subitems;
p_input_item = input_item_New( INPUT_ITEM_URI_NOP, psz_name );
if (!p_input_item)
{
libvlc_printerr( "Not enough memory" );
return NULL;
}
p_md = libvlc_media_new_from_input_item( p_instance, p_input_item );
input_item_Release( p_input_item );
p_subitems = media_get_subitems( p_md, true );
if( p_subitems == NULL) {
libvlc_media_release( p_md );
return NULL;
}
return p_md;
}
void libvlc_media_add_option( libvlc_media_t * p_md,
const char * psz_option )
{
libvlc_media_add_option_flag( p_md, psz_option,
VLC_INPUT_OPTION_UNIQUE|VLC_INPUT_OPTION_TRUSTED );
}
void libvlc_media_add_option_flag( libvlc_media_t * p_md,
const char * ppsz_option,
unsigned i_flags )
{
input_item_AddOption( p_md->p_input_item, ppsz_option, i_flags );
}
void libvlc_media_release( libvlc_media_t *p_md )
{
if (!p_md)
return;
p_md->i_refcount--;
if( p_md->i_refcount > 0 )
return;
uninstall_input_item_observer( p_md );
libvlc_MetadataCancel( p_md->p_libvlc_instance->p_libvlc_int, p_md );
if( p_md->p_subitems )
libvlc_media_list_release( p_md->p_subitems );
input_item_Release( p_md->p_input_item );
libvlc_event_t event;
event.type = libvlc_MediaFreed;
event.u.media_freed.md = p_md;
libvlc_event_send( &p_md->event_manager, &event );
libvlc_event_manager_destroy( &p_md->event_manager );
libvlc_release( p_md->p_libvlc_instance );
free( p_md );
}
void libvlc_media_retain( libvlc_media_t *p_md )
{
assert (p_md);
p_md->i_refcount++;
}
libvlc_media_t *
libvlc_media_duplicate( libvlc_media_t *p_md_orig )
{
return libvlc_media_new_from_input_item(
p_md_orig->p_libvlc_instance, p_md_orig->p_input_item );
}
char *
libvlc_media_get_mrl( libvlc_media_t * p_md )
{
assert( p_md );
return input_item_GetURI( p_md->p_input_item );
}
char *libvlc_media_get_meta( libvlc_media_t *p_md, libvlc_meta_t e_meta )
{
char *psz_meta = NULL;
if( e_meta == libvlc_meta_NowPlaying )
{
psz_meta = input_item_GetNowPlayingFb( p_md->p_input_item );
}
else
{
psz_meta = input_item_GetMeta( p_md->p_input_item,
libvlc_to_vlc_meta[e_meta] );
if( psz_meta == NULL && e_meta == libvlc_meta_Title
&& p_md->p_input_item->psz_name != NULL )
psz_meta = strdup( p_md->p_input_item->psz_name );
}
return psz_meta;
}
void libvlc_media_set_meta( libvlc_media_t *p_md, libvlc_meta_t e_meta, const char *psz_value )
{
assert( p_md );
input_item_SetMeta( p_md->p_input_item, libvlc_to_vlc_meta[e_meta], psz_value );
}
int libvlc_media_save_meta( libvlc_media_t *p_md )
{
assert( p_md );
vlc_object_t *p_obj = VLC_OBJECT(p_md->p_libvlc_instance->p_libvlc_int);
return input_item_WriteMeta( p_obj, p_md->p_input_item ) == VLC_SUCCESS;
}
libvlc_state_t
libvlc_media_get_state( libvlc_media_t *p_md )
{
assert( p_md );
return p_md->state;
}
void
libvlc_media_set_state( libvlc_media_t *p_md,
libvlc_state_t state )
{
libvlc_event_t event;
p_md->state = state;
event.type = libvlc_MediaStateChanged;
event.u.media_state_changed.new_state = state;
libvlc_event_send( &p_md->event_manager, &event );
}
libvlc_media_list_t *
libvlc_media_subitems( libvlc_media_t * p_md )
{
libvlc_media_list_t *p_subitems = media_get_subitems( p_md, true );
if( p_subitems )
libvlc_media_list_retain( p_subitems );
return p_subitems;
}
bool libvlc_media_get_stats(libvlc_media_t *p_md,
libvlc_media_stats_t *p_stats)
{
input_item_t *item = p_md->p_input_item;
if( !p_md->p_input_item )
return false;
vlc_mutex_lock( &item->lock );
input_stats_t *p_itm_stats = p_md->p_input_item->p_stats;
if( p_itm_stats == NULL )
{
vlc_mutex_unlock( &item->lock );
return false;
}
p_stats->i_read_bytes = p_itm_stats->i_read_bytes;
p_stats->f_input_bitrate = p_itm_stats->f_input_bitrate;
p_stats->i_demux_read_bytes = p_itm_stats->i_demux_read_bytes;
p_stats->f_demux_bitrate = p_itm_stats->f_demux_bitrate;
p_stats->i_demux_corrupted = p_itm_stats->i_demux_corrupted;
p_stats->i_demux_discontinuity = p_itm_stats->i_demux_discontinuity;
p_stats->i_decoded_video = p_itm_stats->i_decoded_video;
p_stats->i_decoded_audio = p_itm_stats->i_decoded_audio;
p_stats->i_displayed_pictures = p_itm_stats->i_displayed_pictures;
p_stats->i_lost_pictures = p_itm_stats->i_lost_pictures;
p_stats->i_played_abuffers = p_itm_stats->i_played_abuffers;
p_stats->i_lost_abuffers = p_itm_stats->i_lost_abuffers;
vlc_mutex_unlock( &item->lock );
return true;
}
libvlc_event_manager_t *
libvlc_media_event_manager( libvlc_media_t * p_md )
{
assert( p_md );
return &p_md->event_manager;
}
int64_t
libvlc_media_get_duration( libvlc_media_t * p_md )
{
assert( p_md );
if( !p_md->p_input_item )
{
libvlc_printerr( "No input item" );
return -1;
}
if (!input_item_IsPreparsed( p_md->p_input_item ))
return -1;
return from_mtime(input_item_GetDuration( p_md->p_input_item ));
}
static const input_preparser_callbacks_t input_preparser_callbacks = {
.on_preparse_ended = input_item_preparse_ended,
.on_subtree_added = input_item_subtree_added,
};
static int media_parse(libvlc_media_t *media, bool b_async,
libvlc_media_parse_flag_t parse_flag, int timeout)
{
bool needed;
vlc_mutex_lock(&media->parsed_lock);
needed = !media->has_asked_preparse;
media->has_asked_preparse = true;
if (needed)
{
media->is_parsed = false;
media->parsed_status = 0;
}
vlc_mutex_unlock(&media->parsed_lock);
if (needed)
{
libvlc_int_t *libvlc = media->p_libvlc_instance->p_libvlc_int;
input_item_t *item = media->p_input_item;
input_item_meta_request_option_t parse_scope = META_REQUEST_OPTION_SCOPE_LOCAL;
int ret;
if (parse_flag & libvlc_media_parse_network)
parse_scope |= META_REQUEST_OPTION_SCOPE_NETWORK;
if (parse_flag & libvlc_media_fetch_local)
parse_scope |= META_REQUEST_OPTION_FETCH_LOCAL;
if (parse_flag & libvlc_media_fetch_network)
parse_scope |= META_REQUEST_OPTION_FETCH_NETWORK;
if (parse_flag & libvlc_media_do_interact)
parse_scope |= META_REQUEST_OPTION_DO_INTERACT;
ret = libvlc_MetadataRequest(libvlc, item, parse_scope,
&input_preparser_callbacks, media,
timeout, media);
if (ret != VLC_SUCCESS)
return ret;
}
else
return VLC_EGENERIC;
if (!b_async)
{
vlc_mutex_lock(&media->parsed_lock);
while (!media->is_parsed)
vlc_cond_wait(&media->parsed_cond, &media->parsed_lock);
vlc_mutex_unlock(&media->parsed_lock);
}
return VLC_SUCCESS;
}
void
libvlc_media_parse(libvlc_media_t *media)
{
media_parse( media, false, libvlc_media_fetch_local, -1 );
}
void
libvlc_media_parse_async(libvlc_media_t *media)
{
media_parse( media, true, libvlc_media_fetch_local, -1 );
}
int
libvlc_media_parse_with_options( libvlc_media_t *media,
libvlc_media_parse_flag_t parse_flag,
int timeout )
{
return media_parse( media, true, parse_flag, timeout ) == VLC_SUCCESS ? 0 : -1;
}
void
libvlc_media_parse_stop( libvlc_media_t *media )
{
libvlc_MetadataCancel( media->p_libvlc_instance->p_libvlc_int, media );
}
bool libvlc_media_is_parsed(libvlc_media_t *media)
{
bool parsed;
vlc_mutex_lock(&media->parsed_lock);
parsed = media->is_parsed;
vlc_mutex_unlock(&media->parsed_lock);
return parsed;
}
libvlc_media_parsed_status_t
libvlc_media_get_parsed_status(libvlc_media_t *media)
{
libvlc_media_parsed_status_t status;
vlc_mutex_lock(&media->parsed_lock);
status = media->parsed_status;
vlc_mutex_unlock(&media->parsed_lock);
return status;
}
void
libvlc_media_set_user_data( libvlc_media_t * p_md, void * p_new_user_data )
{
assert( p_md );
p_md->p_user_data = p_new_user_data;
}
void *
libvlc_media_get_user_data( libvlc_media_t * p_md )
{
assert( p_md );
return p_md->p_user_data;
}
unsigned
libvlc_media_tracks_get( libvlc_media_t *p_md, libvlc_media_track_t *** pp_es )
{
assert( p_md );
input_item_t *p_input_item = p_md->p_input_item;
vlc_mutex_lock( &p_input_item->lock );
const int i_es = p_input_item->i_es;
*pp_es = (i_es > 0) ? calloc( i_es, sizeof(**pp_es) ) : NULL;
if( !*pp_es ) 
{
vlc_mutex_unlock( &p_input_item->lock );
return 0;
}
for( int i = 0; i < i_es; i++ )
{
libvlc_media_track_t *p_mes = calloc( 1, sizeof(*p_mes) );
if ( p_mes )
{
p_mes->audio = malloc( __MAX(__MAX(sizeof(*p_mes->audio),
sizeof(*p_mes->video)),
sizeof(*p_mes->subtitle)) );
}
if ( !p_mes || !p_mes->audio )
{
libvlc_media_tracks_release( *pp_es, i_es );
*pp_es = NULL;
free( p_mes );
vlc_mutex_unlock( &p_input_item->lock );
return 0;
}
(*pp_es)[i] = p_mes;
const es_format_t *p_es = p_input_item->es[i];
p_mes->i_codec = p_es->i_codec;
p_mes->i_original_fourcc = p_es->i_original_fourcc;
p_mes->i_id = p_es->i_id;
p_mes->i_profile = p_es->i_profile;
p_mes->i_level = p_es->i_level;
p_mes->i_bitrate = p_es->i_bitrate;
p_mes->psz_language = p_es->psz_language != NULL ? strdup(p_es->psz_language) : NULL;
p_mes->psz_description = p_es->psz_description != NULL ? strdup(p_es->psz_description) : NULL;
switch(p_es->i_cat)
{
case UNKNOWN_ES:
default:
p_mes->i_type = libvlc_track_unknown;
break;
case VIDEO_ES:
p_mes->i_type = libvlc_track_video;
p_mes->video->i_height = p_es->video.i_visible_height;
p_mes->video->i_width = p_es->video.i_visible_width;
p_mes->video->i_sar_num = p_es->video.i_sar_num;
p_mes->video->i_sar_den = p_es->video.i_sar_den;
p_mes->video->i_frame_rate_num = p_es->video.i_frame_rate;
p_mes->video->i_frame_rate_den = p_es->video.i_frame_rate_base;
assert( p_es->video.orientation >= ORIENT_TOP_LEFT &&
p_es->video.orientation <= ORIENT_RIGHT_BOTTOM );
p_mes->video->i_orientation = (int) p_es->video.orientation;
assert( ( p_es->video.projection_mode >= PROJECTION_MODE_RECTANGULAR &&
p_es->video.projection_mode <= PROJECTION_MODE_EQUIRECTANGULAR ) ||
( p_es->video.projection_mode == PROJECTION_MODE_CUBEMAP_LAYOUT_STANDARD ) );
p_mes->video->i_projection = (int) p_es->video.projection_mode;
p_mes->video->pose.f_yaw = p_es->video.pose.yaw;
p_mes->video->pose.f_pitch = p_es->video.pose.pitch;
p_mes->video->pose.f_roll = p_es->video.pose.roll;
p_mes->video->pose.f_field_of_view = p_es->video.pose.fov;
assert( p_es->video.multiview_mode >= MULTIVIEW_2D &&
p_es->video.multiview_mode <= MULTIVIEW_STEREO_CHECKERBOARD );
p_mes->video->i_multiview = (int) p_es->video.multiview_mode;
break;
case AUDIO_ES:
p_mes->i_type = libvlc_track_audio;
p_mes->audio->i_channels = p_es->audio.i_channels;
p_mes->audio->i_rate = p_es->audio.i_rate;
break;
case SPU_ES:
p_mes->i_type = libvlc_track_text;
p_mes->subtitle->psz_encoding = p_es->subs.psz_encoding != NULL ?
strdup(p_es->subs.psz_encoding) : NULL;
break;
}
}
vlc_mutex_unlock( &p_input_item->lock );
return i_es;
}
const char *
libvlc_media_get_codec_description( libvlc_track_type_t i_type,
uint32_t i_codec )
{
switch( i_type )
{
case libvlc_track_audio:
return vlc_fourcc_GetDescription( AUDIO_ES, i_codec );
case libvlc_track_video:
return vlc_fourcc_GetDescription( VIDEO_ES, i_codec );
case libvlc_track_text:
return vlc_fourcc_GetDescription( SPU_ES, i_codec );
case libvlc_track_unknown:
default:
return vlc_fourcc_GetDescription( UNKNOWN_ES, i_codec );
}
}
void libvlc_media_tracks_release( libvlc_media_track_t **p_tracks, unsigned i_count )
{
for( unsigned i = 0; i < i_count; ++i )
{
if ( !p_tracks[i] )
continue;
free( p_tracks[i]->psz_language );
free( p_tracks[i]->psz_description );
switch( p_tracks[i]->i_type )
{
case libvlc_track_audio:
break;
case libvlc_track_video:
break;
case libvlc_track_text:
free( p_tracks[i]->subtitle->psz_encoding );
break;
case libvlc_track_unknown:
default:
break;
}
free( p_tracks[i]->audio );
free( p_tracks[i] );
}
free( p_tracks );
}
libvlc_media_type_t libvlc_media_get_type( libvlc_media_t *p_md )
{
assert( p_md );
enum input_item_type_e i_type;
input_item_t *p_input_item = p_md->p_input_item;
vlc_mutex_lock( &p_input_item->lock );
i_type = p_md->p_input_item->i_type;
vlc_mutex_unlock( &p_input_item->lock );
switch( i_type )
{
case ITEM_TYPE_FILE:
return libvlc_media_type_file;
case ITEM_TYPE_NODE:
case ITEM_TYPE_DIRECTORY:
return libvlc_media_type_directory;
case ITEM_TYPE_DISC:
return libvlc_media_type_disc;
case ITEM_TYPE_STREAM:
return libvlc_media_type_stream;
case ITEM_TYPE_PLAYLIST:
return libvlc_media_type_playlist;
default:
return libvlc_media_type_unknown;
}
}
struct libvlc_media_thumbnail_request_t
{
libvlc_media_t *md;
unsigned int width;
unsigned int height;
bool crop;
libvlc_picture_type_t type;
vlc_thumbnailer_request_t* req;
};
static void media_on_thumbnail_ready( void* data, picture_t* thumbnail )
{
libvlc_media_thumbnail_request_t *req = data;
libvlc_media_t *p_media = req->md;
libvlc_event_t event;
event.type = libvlc_MediaThumbnailGenerated;
libvlc_picture_t* pic = NULL;
if ( thumbnail != NULL )
pic = libvlc_picture_new( VLC_OBJECT(p_media->p_libvlc_instance->p_libvlc_int),
thumbnail, req->type, req->width, req->height,
req->crop );
event.u.media_thumbnail_generated.p_thumbnail = pic;
libvlc_event_send( &p_media->event_manager, &event );
if ( pic != NULL )
libvlc_picture_release( pic );
}
libvlc_media_thumbnail_request_t*
libvlc_media_thumbnail_request_by_time( libvlc_media_t *md, libvlc_time_t time,
libvlc_thumbnailer_seek_speed_t speed,
unsigned int width, unsigned int height,
bool crop, libvlc_picture_type_t picture_type,
libvlc_time_t timeout )
{
assert( md );
libvlc_priv_t *p_priv = libvlc_priv(md->p_libvlc_instance->p_libvlc_int);
if( unlikely( p_priv->p_thumbnailer == NULL ) )
return NULL;
libvlc_media_thumbnail_request_t *req = malloc( sizeof( *req ) );
if ( unlikely( req == NULL ) )
return NULL;
req->md = md;
req->width = width;
req->height = height;
req->type = picture_type;
req->crop = crop;
libvlc_media_retain( md );
req->req = vlc_thumbnailer_RequestByTime( p_priv->p_thumbnailer,
VLC_TICK_FROM_MS( time ),
speed == libvlc_media_thumbnail_seek_fast ?
VLC_THUMBNAILER_SEEK_FAST : VLC_THUMBNAILER_SEEK_PRECISE,
md->p_input_item,
timeout > 0 ? VLC_TICK_FROM_MS( timeout ) : VLC_TICK_INVALID,
media_on_thumbnail_ready, req );
if ( req->req == NULL )
{
free( req );
libvlc_media_release( md );
return NULL;
}
return req;
}
libvlc_media_thumbnail_request_t*
libvlc_media_thumbnail_request_by_pos( libvlc_media_t *md, float pos,
libvlc_thumbnailer_seek_speed_t speed,
unsigned int width, unsigned int height,
bool crop, libvlc_picture_type_t picture_type,
libvlc_time_t timeout )
{
assert( md );
libvlc_priv_t *priv = libvlc_priv(md->p_libvlc_instance->p_libvlc_int);
if( unlikely( priv->p_thumbnailer == NULL ) )
return NULL;
libvlc_media_thumbnail_request_t *req = malloc( sizeof( *req ) );
if ( unlikely( req == NULL ) )
return NULL;
req->md = md;
req->width = width;
req->height = height;
req->crop = crop;
req->type = picture_type;
libvlc_media_retain( md );
req->req = vlc_thumbnailer_RequestByPos( priv->p_thumbnailer, pos,
speed == libvlc_media_thumbnail_seek_fast ?
VLC_THUMBNAILER_SEEK_FAST : VLC_THUMBNAILER_SEEK_PRECISE,
md->p_input_item,
timeout > 0 ? VLC_TICK_FROM_MS( timeout ) : VLC_TICK_INVALID,
media_on_thumbnail_ready, req );
if ( req->req == NULL )
{
free( req );
libvlc_media_release( md );
return NULL;
}
return req;
}
void libvlc_media_thumbnail_request_cancel( libvlc_media_thumbnail_request_t *req )
{
libvlc_priv_t *p_priv = libvlc_priv(req->md->p_libvlc_instance->p_libvlc_int);
assert( p_priv->p_thumbnailer != NULL );
vlc_thumbnailer_Cancel( p_priv->p_thumbnailer, req->req );
}
void libvlc_media_thumbnail_request_destroy( libvlc_media_thumbnail_request_t *req )
{
libvlc_media_release( req->md );
free( req );
}
int libvlc_media_slaves_add( libvlc_media_t *p_md,
libvlc_media_slave_type_t i_type,
unsigned int i_priority,
const char *psz_uri )
{
assert( p_md && psz_uri );
input_item_t *p_input_item = p_md->p_input_item;
enum slave_type i_input_slave_type;
switch( i_type )
{
case libvlc_media_slave_type_subtitle:
i_input_slave_type = SLAVE_TYPE_SPU;
break;
case libvlc_media_slave_type_audio:
i_input_slave_type = SLAVE_TYPE_AUDIO;
break;
default:
vlc_assert_unreachable();
return -1;
}
enum slave_priority i_input_slave_priority;
switch( i_priority )
{
case 0:
i_input_slave_priority = SLAVE_PRIORITY_MATCH_NONE;
break;
case 1:
i_input_slave_priority = SLAVE_PRIORITY_MATCH_RIGHT;
break;
case 2:
i_input_slave_priority = SLAVE_PRIORITY_MATCH_LEFT;
break;
case 3:
i_input_slave_priority = SLAVE_PRIORITY_MATCH_ALL;
break;
default:
case 4:
i_input_slave_priority = SLAVE_PRIORITY_USER;
break;
}
input_item_slave_t *p_slave = input_item_slave_New( psz_uri,
i_input_slave_type,
i_input_slave_priority );
if( p_slave == NULL )
return -1;
return input_item_AddSlave( p_input_item, p_slave ) == VLC_SUCCESS ? 0 : -1;
}
void libvlc_media_slaves_clear( libvlc_media_t *p_md )
{
assert( p_md );
input_item_t *p_input_item = p_md->p_input_item;
vlc_mutex_lock( &p_input_item->lock );
for( int i = 0; i < p_input_item->i_slaves; i++ )
input_item_slave_Delete( p_input_item->pp_slaves[i] );
TAB_CLEAN( p_input_item->i_slaves, p_input_item->pp_slaves );
vlc_mutex_unlock( &p_input_item->lock );
}
unsigned int libvlc_media_slaves_get( libvlc_media_t *p_md,
libvlc_media_slave_t ***ppp_slaves )
{
assert( p_md && ppp_slaves );
input_item_t *p_input_item = p_md->p_input_item;
*ppp_slaves = NULL;
vlc_mutex_lock( &p_input_item->lock );
int i_count = p_input_item->i_slaves;
if( i_count <= 0 )
return vlc_mutex_unlock( &p_input_item->lock ), 0;
libvlc_media_slave_t **pp_slaves = calloc( i_count, sizeof(*pp_slaves) );
if( pp_slaves == NULL )
return vlc_mutex_unlock( &p_input_item->lock ), 0;
for( int i = 0; i < i_count; ++i )
{
input_item_slave_t *p_item_slave = p_input_item->pp_slaves[i];
assert( p_item_slave->i_priority >= SLAVE_PRIORITY_MATCH_NONE );
libvlc_media_slave_t *p_slave = malloc( sizeof(*p_slave) +
strlen( p_item_slave->psz_uri )
+ 1 );
if( p_slave == NULL )
{
libvlc_media_slaves_release(pp_slaves, i);
return vlc_mutex_unlock( &p_input_item->lock ), 0;
}
p_slave->psz_uri = (char *) ((uint8_t *)p_slave) + sizeof(*p_slave);
strcpy( p_slave->psz_uri, p_item_slave->psz_uri );
switch( p_item_slave->i_type )
{
case SLAVE_TYPE_SPU:
p_slave->i_type = libvlc_media_slave_type_subtitle;
break;
case SLAVE_TYPE_AUDIO:
p_slave->i_type = libvlc_media_slave_type_audio;
break;
default:
vlc_assert_unreachable();
}
switch( p_item_slave->i_priority )
{
case SLAVE_PRIORITY_MATCH_NONE:
p_slave->i_priority = 0;
break;
case SLAVE_PRIORITY_MATCH_RIGHT:
p_slave->i_priority = 1;
break;
case SLAVE_PRIORITY_MATCH_LEFT:
p_slave->i_priority = 2;
break;
case SLAVE_PRIORITY_MATCH_ALL:
p_slave->i_priority = 3;
break;
case SLAVE_PRIORITY_USER:
p_slave->i_priority = 4;
break;
default:
vlc_assert_unreachable();
}
pp_slaves[i] = p_slave;
}
vlc_mutex_unlock( &p_input_item->lock );
*ppp_slaves = pp_slaves;
return i_count;
}
void libvlc_media_slaves_release( libvlc_media_slave_t **pp_slaves,
unsigned int i_count )
{
if( i_count > 0 )
{
assert( pp_slaves );
for( unsigned int i = 0; i < i_count; ++i )
free( pp_slaves[i] );
}
free( pp_slaves );
}
