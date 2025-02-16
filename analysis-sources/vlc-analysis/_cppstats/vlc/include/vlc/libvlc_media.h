#if defined(__cplusplus)
extern "C" {
#else
#include <stdbool.h>
#endif
typedef struct libvlc_media_t libvlc_media_t;
typedef enum libvlc_meta_t {
libvlc_meta_Title,
libvlc_meta_Artist,
libvlc_meta_Genre,
libvlc_meta_Copyright,
libvlc_meta_Album,
libvlc_meta_TrackNumber,
libvlc_meta_Description,
libvlc_meta_Rating,
libvlc_meta_Date,
libvlc_meta_Setting,
libvlc_meta_URL,
libvlc_meta_Language,
libvlc_meta_NowPlaying,
libvlc_meta_Publisher,
libvlc_meta_EncodedBy,
libvlc_meta_ArtworkURL,
libvlc_meta_TrackID,
libvlc_meta_TrackTotal,
libvlc_meta_Director,
libvlc_meta_Season,
libvlc_meta_Episode,
libvlc_meta_ShowName,
libvlc_meta_Actors,
libvlc_meta_AlbumArtist,
libvlc_meta_DiscNumber,
libvlc_meta_DiscTotal
} libvlc_meta_t;
typedef enum libvlc_state_t
{
libvlc_NothingSpecial=0,
libvlc_Opening,
libvlc_Buffering, 
libvlc_Playing,
libvlc_Paused,
libvlc_Stopped,
libvlc_Ended,
libvlc_Error
} libvlc_state_t;
enum
{
libvlc_media_option_trusted = 0x2,
libvlc_media_option_unique = 0x100
};
typedef enum libvlc_track_type_t
{
libvlc_track_unknown = -1,
libvlc_track_audio = 0,
libvlc_track_video = 1,
libvlc_track_text = 2
} libvlc_track_type_t;
typedef struct libvlc_media_stats_t
{
int i_read_bytes;
float f_input_bitrate;
int i_demux_read_bytes;
float f_demux_bitrate;
int i_demux_corrupted;
int i_demux_discontinuity;
int i_decoded_video;
int i_decoded_audio;
int i_displayed_pictures;
int i_lost_pictures;
int i_played_abuffers;
int i_lost_abuffers;
} libvlc_media_stats_t;
typedef struct libvlc_audio_track_t
{
unsigned i_channels;
unsigned i_rate;
} libvlc_audio_track_t;
typedef enum libvlc_video_orient_t
{
libvlc_video_orient_top_left, 
libvlc_video_orient_top_right, 
libvlc_video_orient_bottom_left, 
libvlc_video_orient_bottom_right, 
libvlc_video_orient_left_top, 
libvlc_video_orient_left_bottom, 
libvlc_video_orient_right_top, 
libvlc_video_orient_right_bottom 
} libvlc_video_orient_t;
typedef enum libvlc_video_projection_t
{
libvlc_video_projection_rectangular,
libvlc_video_projection_equirectangular, 
libvlc_video_projection_cubemap_layout_standard = 0x100,
} libvlc_video_projection_t;
typedef struct libvlc_video_viewpoint_t
{
float f_yaw; 
float f_pitch; 
float f_roll; 
float f_field_of_view; 
} libvlc_video_viewpoint_t;
typedef enum libvlc_video_multiview_t
{
libvlc_video_multiview_2d, 
libvlc_video_multiview_stereo_sbs, 
libvlc_video_multiview_stereo_tb, 
libvlc_video_multiview_stereo_row, 
libvlc_video_multiview_stereo_col, 
libvlc_video_multiview_stereo_frame, 
libvlc_video_multiview_stereo_checkerboard, 
} libvlc_video_multiview_t;
typedef struct libvlc_video_track_t
{
unsigned i_height;
unsigned i_width;
unsigned i_sar_num;
unsigned i_sar_den;
unsigned i_frame_rate_num;
unsigned i_frame_rate_den;
libvlc_video_orient_t i_orientation;
libvlc_video_projection_t i_projection;
libvlc_video_viewpoint_t pose; 
libvlc_video_multiview_t i_multiview;
} libvlc_video_track_t;
typedef struct libvlc_subtitle_track_t
{
char *psz_encoding;
} libvlc_subtitle_track_t;
typedef struct libvlc_media_track_t
{
uint32_t i_codec;
uint32_t i_original_fourcc;
int i_id;
libvlc_track_type_t i_type;
int i_profile;
int i_level;
union {
libvlc_audio_track_t *audio;
libvlc_video_track_t *video;
libvlc_subtitle_track_t *subtitle;
};
unsigned int i_bitrate;
char *psz_language;
char *psz_description;
} libvlc_media_track_t;
typedef enum libvlc_media_type_t {
libvlc_media_type_unknown,
libvlc_media_type_file,
libvlc_media_type_directory,
libvlc_media_type_disc,
libvlc_media_type_stream,
libvlc_media_type_playlist,
} libvlc_media_type_t;
typedef enum libvlc_media_parse_flag_t
{
libvlc_media_parse_local = 0x00,
libvlc_media_parse_network = 0x01,
libvlc_media_fetch_local = 0x02,
libvlc_media_fetch_network = 0x04,
libvlc_media_do_interact = 0x08,
} libvlc_media_parse_flag_t;
typedef enum libvlc_media_parsed_status_t
{
libvlc_media_parsed_status_skipped = 1,
libvlc_media_parsed_status_failed,
libvlc_media_parsed_status_timeout,
libvlc_media_parsed_status_done,
} libvlc_media_parsed_status_t;
typedef enum libvlc_media_slave_type_t
{
libvlc_media_slave_type_subtitle,
libvlc_media_slave_type_audio,
} libvlc_media_slave_type_t;
typedef struct libvlc_media_slave_t
{
char * psz_uri;
libvlc_media_slave_type_t i_type;
unsigned int i_priority;
} libvlc_media_slave_t;
typedef int (*libvlc_media_open_cb)(void *opaque, void **datap,
uint64_t *sizep);
typedef ssize_t (*libvlc_media_read_cb)(void *opaque, unsigned char *buf,
size_t len);
typedef int (*libvlc_media_seek_cb)(void *opaque, uint64_t offset);
typedef void (*libvlc_media_close_cb)(void *opaque);
LIBVLC_API libvlc_media_t *libvlc_media_new_location(
libvlc_instance_t *p_instance,
const char * psz_mrl );
LIBVLC_API libvlc_media_t *libvlc_media_new_path(
libvlc_instance_t *p_instance,
const char *path );
LIBVLC_API libvlc_media_t *libvlc_media_new_fd(
libvlc_instance_t *p_instance,
int fd );
LIBVLC_API libvlc_media_t *libvlc_media_new_callbacks(
libvlc_instance_t *instance,
libvlc_media_open_cb open_cb,
libvlc_media_read_cb read_cb,
libvlc_media_seek_cb seek_cb,
libvlc_media_close_cb close_cb,
void *opaque );
LIBVLC_API libvlc_media_t *libvlc_media_new_as_node(
libvlc_instance_t *p_instance,
const char * psz_name );
LIBVLC_API void libvlc_media_add_option(
libvlc_media_t *p_md,
const char * psz_options );
LIBVLC_API void libvlc_media_add_option_flag(
libvlc_media_t *p_md,
const char * psz_options,
unsigned i_flags );
LIBVLC_API void libvlc_media_retain( libvlc_media_t *p_md );
LIBVLC_API void libvlc_media_release( libvlc_media_t *p_md );
LIBVLC_API char *libvlc_media_get_mrl( libvlc_media_t *p_md );
LIBVLC_API libvlc_media_t *libvlc_media_duplicate( libvlc_media_t *p_md );
LIBVLC_API char *libvlc_media_get_meta( libvlc_media_t *p_md,
libvlc_meta_t e_meta );
LIBVLC_API void libvlc_media_set_meta( libvlc_media_t *p_md,
libvlc_meta_t e_meta,
const char *psz_value );
LIBVLC_API int libvlc_media_save_meta( libvlc_media_t *p_md );
LIBVLC_API libvlc_state_t libvlc_media_get_state(
libvlc_media_t *p_md );
LIBVLC_API bool libvlc_media_get_stats(libvlc_media_t *p_md,
libvlc_media_stats_t *p_stats);
#define VLC_FORWARD_DECLARE_OBJECT(a) struct a
LIBVLC_API VLC_FORWARD_DECLARE_OBJECT(libvlc_media_list_t *)
libvlc_media_subitems( libvlc_media_t *p_md );
LIBVLC_API libvlc_event_manager_t *
libvlc_media_event_manager( libvlc_media_t *p_md );
LIBVLC_API libvlc_time_t
libvlc_media_get_duration( libvlc_media_t *p_md );
LIBVLC_API int
libvlc_media_parse_with_options( libvlc_media_t *p_md,
libvlc_media_parse_flag_t parse_flag,
int timeout );
LIBVLC_API void
libvlc_media_parse_stop( libvlc_media_t *p_md );
LIBVLC_API libvlc_media_parsed_status_t
libvlc_media_get_parsed_status( libvlc_media_t *p_md );
LIBVLC_API void
libvlc_media_set_user_data( libvlc_media_t *p_md, void *p_new_user_data );
LIBVLC_API void *libvlc_media_get_user_data( libvlc_media_t *p_md );
LIBVLC_API
unsigned libvlc_media_tracks_get( libvlc_media_t *p_md,
libvlc_media_track_t ***tracks );
LIBVLC_API
const char *libvlc_media_get_codec_description( libvlc_track_type_t i_type,
uint32_t i_codec );
LIBVLC_API
void libvlc_media_tracks_release( libvlc_media_track_t **p_tracks,
unsigned i_count );
LIBVLC_API
libvlc_media_type_t libvlc_media_get_type( libvlc_media_t *p_md );
typedef struct libvlc_media_thumbnail_request_t libvlc_media_thumbnail_request_t;
typedef enum libvlc_thumbnailer_seek_speed_t
{
libvlc_media_thumbnail_seek_precise,
libvlc_media_thumbnail_seek_fast,
} libvlc_thumbnailer_seek_speed_t;
LIBVLC_API libvlc_media_thumbnail_request_t*
libvlc_media_thumbnail_request_by_time( libvlc_media_t *md,
libvlc_time_t time,
libvlc_thumbnailer_seek_speed_t speed,
unsigned int width, unsigned int height,
bool crop, libvlc_picture_type_t picture_type,
libvlc_time_t timeout );
LIBVLC_API libvlc_media_thumbnail_request_t*
libvlc_media_thumbnail_request_by_pos( libvlc_media_t *md,
float pos,
libvlc_thumbnailer_seek_speed_t speed,
unsigned int width, unsigned int height,
bool crop, libvlc_picture_type_t picture_type,
libvlc_time_t timeout );
LIBVLC_API void
libvlc_media_thumbnail_request_cancel( libvlc_media_thumbnail_request_t *p_req );
LIBVLC_API void
libvlc_media_thumbnail_request_destroy( libvlc_media_thumbnail_request_t *p_req );
LIBVLC_API
int libvlc_media_slaves_add( libvlc_media_t *p_md,
libvlc_media_slave_type_t i_type,
unsigned int i_priority,
const char *psz_uri );
LIBVLC_API
void libvlc_media_slaves_clear( libvlc_media_t *p_md );
LIBVLC_API
unsigned int libvlc_media_slaves_get( libvlc_media_t *p_md,
libvlc_media_slave_t ***ppp_slaves );
LIBVLC_API
void libvlc_media_slaves_release( libvlc_media_slave_t **pp_slaves,
unsigned int i_count );
#if defined(__cplusplus)
}
#endif
