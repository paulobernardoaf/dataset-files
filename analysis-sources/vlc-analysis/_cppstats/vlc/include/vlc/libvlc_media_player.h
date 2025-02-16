#if defined(__cplusplus)
extern "C" {
#else
#include <stdbool.h>
#endif
typedef struct libvlc_media_player_t libvlc_media_player_t;
typedef struct libvlc_track_description_t
{
int i_id;
char *psz_name;
struct libvlc_track_description_t *p_next;
} libvlc_track_description_t;
enum
{
libvlc_title_menu = 0x01,
libvlc_title_interactive = 0x02
};
typedef struct libvlc_title_description_t
{
int64_t i_duration; 
char *psz_name; 
unsigned i_flags; 
} libvlc_title_description_t;
typedef struct libvlc_chapter_description_t
{
int64_t i_time_offset; 
int64_t i_duration; 
char *psz_name; 
} libvlc_chapter_description_t;
typedef struct libvlc_audio_output_t
{
char *psz_name;
char *psz_description;
struct libvlc_audio_output_t *p_next;
} libvlc_audio_output_t;
typedef struct libvlc_audio_output_device_t
{
struct libvlc_audio_output_device_t *p_next; 
char *psz_device; 
char *psz_description; 
} libvlc_audio_output_device_t;
typedef enum libvlc_video_marquee_option_t {
libvlc_marquee_Enable = 0,
libvlc_marquee_Text, 
libvlc_marquee_Color,
libvlc_marquee_Opacity,
libvlc_marquee_Position,
libvlc_marquee_Refresh,
libvlc_marquee_Size,
libvlc_marquee_Timeout,
libvlc_marquee_X,
libvlc_marquee_Y
} libvlc_video_marquee_option_t;
typedef enum libvlc_navigate_mode_t
{
libvlc_navigate_activate = 0,
libvlc_navigate_up,
libvlc_navigate_down,
libvlc_navigate_left,
libvlc_navigate_right,
libvlc_navigate_popup
} libvlc_navigate_mode_t;
typedef enum libvlc_position_t {
libvlc_position_disable=-1,
libvlc_position_center,
libvlc_position_left,
libvlc_position_right,
libvlc_position_top,
libvlc_position_top_left,
libvlc_position_top_right,
libvlc_position_bottom,
libvlc_position_bottom_left,
libvlc_position_bottom_right
} libvlc_position_t;
typedef enum libvlc_teletext_key_t {
libvlc_teletext_key_red = 'r' << 16,
libvlc_teletext_key_green = 'g' << 16,
libvlc_teletext_key_yellow = 'y' << 16,
libvlc_teletext_key_blue = 'b' << 16,
libvlc_teletext_key_index = 'i' << 16,
} libvlc_teletext_key_t;
typedef struct libvlc_equalizer_t libvlc_equalizer_t;
LIBVLC_API libvlc_media_player_t * libvlc_media_player_new( libvlc_instance_t *p_libvlc_instance );
LIBVLC_API libvlc_media_player_t * libvlc_media_player_new_from_media( libvlc_media_t *p_md );
LIBVLC_API void libvlc_media_player_release( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_retain( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_set_media( libvlc_media_player_t *p_mi,
libvlc_media_t *p_md );
LIBVLC_API libvlc_media_t * libvlc_media_player_get_media( libvlc_media_player_t *p_mi );
LIBVLC_API libvlc_event_manager_t * libvlc_media_player_event_manager ( libvlc_media_player_t *p_mi );
LIBVLC_API bool libvlc_media_player_is_playing(libvlc_media_player_t *p_mi);
LIBVLC_API int libvlc_media_player_play ( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_set_pause ( libvlc_media_player_t *mp,
int do_pause );
LIBVLC_API void libvlc_media_player_pause ( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_stop_async ( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_set_renderer( libvlc_media_player_t *p_mi,
libvlc_renderer_item_t *p_item );
typedef enum libvlc_video_color_primaries_t {
libvlc_video_primaries_BT601_525 = 1,
libvlc_video_primaries_BT601_625 = 2,
libvlc_video_primaries_BT709 = 3,
libvlc_video_primaries_BT2020 = 4,
libvlc_video_primaries_DCI_P3 = 5,
libvlc_video_primaries_BT470_M = 6,
} libvlc_video_color_primaries_t;
typedef enum libvlc_video_color_space_t {
libvlc_video_colorspace_BT601 = 1,
libvlc_video_colorspace_BT709 = 2,
libvlc_video_colorspace_BT2020 = 3,
} libvlc_video_color_space_t;
typedef enum libvlc_video_transfer_func_t {
libvlc_video_transfer_func_LINEAR = 1,
libvlc_video_transfer_func_SRGB = 2,
libvlc_video_transfer_func_BT470_BG = 3,
libvlc_video_transfer_func_BT470_M = 4,
libvlc_video_transfer_func_BT709 = 5,
libvlc_video_transfer_func_PQ = 6,
libvlc_video_transfer_func_SMPTE_240 = 7,
libvlc_video_transfer_func_HLG = 8,
} libvlc_video_transfer_func_t;
typedef void *(*libvlc_video_lock_cb)(void *opaque, void **planes);
typedef void (*libvlc_video_unlock_cb)(void *opaque, void *picture,
void *const *planes);
typedef void (*libvlc_video_display_cb)(void *opaque, void *picture);
typedef unsigned (*libvlc_video_format_cb)(void **opaque, char *chroma,
unsigned *width, unsigned *height,
unsigned *pitches,
unsigned *lines);
typedef void (*libvlc_video_cleanup_cb)(void *opaque);
LIBVLC_API
void libvlc_video_set_callbacks( libvlc_media_player_t *mp,
libvlc_video_lock_cb lock,
libvlc_video_unlock_cb unlock,
libvlc_video_display_cb display,
void *opaque );
LIBVLC_API
void libvlc_video_set_format( libvlc_media_player_t *mp, const char *chroma,
unsigned width, unsigned height,
unsigned pitch );
LIBVLC_API
void libvlc_video_set_format_callbacks( libvlc_media_player_t *mp,
libvlc_video_format_cb setup,
libvlc_video_cleanup_cb cleanup );
typedef struct
{
bool hardware_decoding; 
} libvlc_video_setup_device_cfg_t;
typedef struct
{
union {
struct {
void *device_context; 
} d3d11;
struct {
void *device; 
int adapter; 
} d3d9;
};
} libvlc_video_setup_device_info_t;
typedef bool (*libvlc_video_output_setup_cb)(void **opaque,
const libvlc_video_setup_device_cfg_t *cfg,
libvlc_video_setup_device_info_t *out);
typedef void (*libvlc_video_output_cleanup_cb)(void* opaque);
typedef struct
{
unsigned width; 
unsigned height; 
unsigned bitdepth; 
bool full_range; 
libvlc_video_color_space_t colorspace; 
libvlc_video_color_primaries_t primaries; 
libvlc_video_transfer_func_t transfer; 
void *device; 
} libvlc_video_render_cfg_t;
typedef struct
{
union {
int dxgi_format; 
uint32_t d3d9_format; 
int opengl_format; 
void *p_surface; 
};
bool full_range; 
libvlc_video_color_space_t colorspace; 
libvlc_video_color_primaries_t primaries; 
libvlc_video_transfer_func_t transfer; 
} libvlc_video_output_cfg_t;
typedef bool (*libvlc_video_update_output_cb)(void* opaque, const libvlc_video_render_cfg_t *cfg,
libvlc_video_output_cfg_t *output );
typedef void (*libvlc_video_swap_cb)(void* opaque);
typedef bool (*libvlc_video_makeCurrent_cb)(void* opaque, bool enter);
typedef void* (*libvlc_video_getProcAddress_cb)(void* opaque, const char* fct_name);
typedef struct
{
uint16_t RedPrimary[2];
uint16_t GreenPrimary[2];
uint16_t BluePrimary[2];
uint16_t WhitePoint[2];
unsigned int MaxMasteringLuminance;
unsigned int MinMasteringLuminance;
uint16_t MaxContentLightLevel;
uint16_t MaxFrameAverageLightLevel;
} libvlc_video_frame_hdr10_metadata_t;
typedef enum libvlc_video_metadata_type_t {
libvlc_video_metadata_frame_hdr10, 
} libvlc_video_metadata_type_t;
typedef void (*libvlc_video_frameMetadata_cb)(void* opaque, libvlc_video_metadata_type_t type, const void *metadata);
typedef enum libvlc_video_engine_t {
libvlc_video_engine_disable,
libvlc_video_engine_opengl,
libvlc_video_engine_gles2,
libvlc_video_engine_d3d11,
libvlc_video_engine_d3d9,
} libvlc_video_engine_t;
typedef void( *libvlc_video_output_set_resize_cb )( void *opaque,
void (*report_size_change)(void *report_opaque, unsigned width, unsigned height),
void *report_opaque );
typedef bool( *libvlc_video_output_select_plane_cb )( void *opaque, size_t plane );
LIBVLC_API
bool libvlc_video_set_output_callbacks( libvlc_media_player_t *mp,
libvlc_video_engine_t engine,
libvlc_video_output_setup_cb setup_cb,
libvlc_video_output_cleanup_cb cleanup_cb,
libvlc_video_output_set_resize_cb resize_cb,
libvlc_video_update_output_cb update_output_cb,
libvlc_video_swap_cb swap_cb,
libvlc_video_makeCurrent_cb makeCurrent_cb,
libvlc_video_getProcAddress_cb getProcAddress_cb,
libvlc_video_frameMetadata_cb metadata_cb,
libvlc_video_output_select_plane_cb select_plane_cb,
void* opaque );
LIBVLC_API void libvlc_media_player_set_nsobject ( libvlc_media_player_t *p_mi, void * drawable );
LIBVLC_API void * libvlc_media_player_get_nsobject ( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_set_xwindow(libvlc_media_player_t *p_mi,
uint32_t drawable);
LIBVLC_API uint32_t libvlc_media_player_get_xwindow ( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_set_hwnd ( libvlc_media_player_t *p_mi, void *drawable );
LIBVLC_API void *libvlc_media_player_get_hwnd ( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_set_android_context( libvlc_media_player_t *p_mi,
void *p_awindow_handler );
typedef void (*libvlc_audio_play_cb)(void *data, const void *samples,
unsigned count, int64_t pts);
typedef void (*libvlc_audio_pause_cb)(void *data, int64_t pts);
typedef void (*libvlc_audio_resume_cb)(void *data, int64_t pts);
typedef void (*libvlc_audio_flush_cb)(void *data, int64_t pts);
typedef void (*libvlc_audio_drain_cb)(void *data);
typedef void (*libvlc_audio_set_volume_cb)(void *data,
float volume, bool mute);
LIBVLC_API
void libvlc_audio_set_callbacks( libvlc_media_player_t *mp,
libvlc_audio_play_cb play,
libvlc_audio_pause_cb pause,
libvlc_audio_resume_cb resume,
libvlc_audio_flush_cb flush,
libvlc_audio_drain_cb drain,
void *opaque );
LIBVLC_API
void libvlc_audio_set_volume_callback( libvlc_media_player_t *mp,
libvlc_audio_set_volume_cb set_volume );
typedef int (*libvlc_audio_setup_cb)(void **opaque, char *format, unsigned *rate,
unsigned *channels);
typedef void (*libvlc_audio_cleanup_cb)(void *data);
LIBVLC_API
void libvlc_audio_set_format_callbacks( libvlc_media_player_t *mp,
libvlc_audio_setup_cb setup,
libvlc_audio_cleanup_cb cleanup );
LIBVLC_API
void libvlc_audio_set_format( libvlc_media_player_t *mp, const char *format,
unsigned rate, unsigned channels );
LIBVLC_API libvlc_time_t libvlc_media_player_get_length( libvlc_media_player_t *p_mi );
LIBVLC_API libvlc_time_t libvlc_media_player_get_time( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_set_time( libvlc_media_player_t *p_mi,
libvlc_time_t i_time, bool b_fast );
LIBVLC_API float libvlc_media_player_get_position( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_set_position( libvlc_media_player_t *p_mi,
float f_pos, bool b_fast );
LIBVLC_API void libvlc_media_player_set_chapter( libvlc_media_player_t *p_mi, int i_chapter );
LIBVLC_API int libvlc_media_player_get_chapter( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_get_chapter_count( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_get_chapter_count_for_title(
libvlc_media_player_t *p_mi, int i_title );
LIBVLC_API void libvlc_media_player_set_title( libvlc_media_player_t *p_mi, int i_title );
LIBVLC_API int libvlc_media_player_get_title( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_get_title_count( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_previous_chapter( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_next_chapter( libvlc_media_player_t *p_mi );
LIBVLC_API float libvlc_media_player_get_rate( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_media_player_set_rate( libvlc_media_player_t *p_mi, float rate );
LIBVLC_API libvlc_state_t libvlc_media_player_get_state( libvlc_media_player_t *p_mi );
LIBVLC_API unsigned libvlc_media_player_has_vout( libvlc_media_player_t *p_mi );
LIBVLC_API bool libvlc_media_player_is_seekable(libvlc_media_player_t *p_mi);
LIBVLC_API bool libvlc_media_player_can_pause(libvlc_media_player_t *p_mi);
LIBVLC_API bool libvlc_media_player_program_scrambled( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_next_frame( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_media_player_navigate( libvlc_media_player_t* p_mi,
unsigned navigate );
LIBVLC_API void libvlc_media_player_set_video_title_display( libvlc_media_player_t *p_mi, libvlc_position_t position, unsigned int timeout );
LIBVLC_API
int libvlc_media_player_add_slave( libvlc_media_player_t *p_mi,
libvlc_media_slave_type_t i_type,
const char *psz_uri, bool b_select );
LIBVLC_API void libvlc_track_description_list_release( libvlc_track_description_t *p_track_description );
LIBVLC_API void libvlc_toggle_fullscreen( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_set_fullscreen(libvlc_media_player_t *p_mi, bool b_fullscreen);
LIBVLC_API bool libvlc_get_fullscreen( libvlc_media_player_t *p_mi );
LIBVLC_API
void libvlc_video_set_key_input( libvlc_media_player_t *p_mi, unsigned on );
LIBVLC_API
void libvlc_video_set_mouse_input( libvlc_media_player_t *p_mi, unsigned on );
LIBVLC_API
int libvlc_video_get_size( libvlc_media_player_t *p_mi, unsigned num,
unsigned *px, unsigned *py );
LIBVLC_API
int libvlc_video_get_cursor( libvlc_media_player_t *p_mi, unsigned num,
int *px, int *py );
LIBVLC_API float libvlc_video_get_scale( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_video_set_scale( libvlc_media_player_t *p_mi, float f_factor );
LIBVLC_API char *libvlc_video_get_aspect_ratio( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_video_set_aspect_ratio( libvlc_media_player_t *p_mi, const char *psz_aspect );
LIBVLC_API libvlc_video_viewpoint_t *libvlc_video_new_viewpoint(void);
LIBVLC_API int libvlc_video_update_viewpoint( libvlc_media_player_t *p_mi,
const libvlc_video_viewpoint_t *p_viewpoint,
bool b_absolute);
LIBVLC_API int libvlc_video_get_spu( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_video_get_spu_count( libvlc_media_player_t *p_mi );
LIBVLC_API libvlc_track_description_t *
libvlc_video_get_spu_description( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_video_set_spu( libvlc_media_player_t *p_mi, int i_spu );
LIBVLC_API int64_t libvlc_video_get_spu_delay( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_video_set_spu_delay( libvlc_media_player_t *p_mi, int64_t i_delay );
LIBVLC_API int libvlc_media_player_get_full_title_descriptions( libvlc_media_player_t *p_mi,
libvlc_title_description_t ***titles );
LIBVLC_API
void libvlc_title_descriptions_release( libvlc_title_description_t **p_titles,
unsigned i_count );
LIBVLC_API int libvlc_media_player_get_full_chapter_descriptions( libvlc_media_player_t *p_mi,
int i_chapters_of_title,
libvlc_chapter_description_t *** pp_chapters );
LIBVLC_API
void libvlc_chapter_descriptions_release( libvlc_chapter_description_t **p_chapters,
unsigned i_count );
LIBVLC_API
void libvlc_video_set_crop_ratio(libvlc_media_player_t *mp,
unsigned num, unsigned den);
LIBVLC_API
void libvlc_video_set_crop_window(libvlc_media_player_t *mp,
unsigned x, unsigned y,
unsigned width, unsigned height);
LIBVLC_API
void libvlc_video_set_crop_border(libvlc_media_player_t *mp,
unsigned left, unsigned right,
unsigned top, unsigned bottom);
LIBVLC_API int libvlc_video_get_teletext( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_video_set_teletext( libvlc_media_player_t *p_mi, int i_page );
LIBVLC_API int libvlc_video_get_track_count( libvlc_media_player_t *p_mi );
LIBVLC_API libvlc_track_description_t *
libvlc_video_get_track_description( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_video_get_track( libvlc_media_player_t *p_mi );
LIBVLC_API
int libvlc_video_set_track( libvlc_media_player_t *p_mi, int i_track );
LIBVLC_API
int libvlc_video_take_snapshot( libvlc_media_player_t *p_mi, unsigned num,
const char *psz_filepath, unsigned int i_width,
unsigned int i_height );
LIBVLC_API void libvlc_video_set_deinterlace( libvlc_media_player_t *p_mi,
int deinterlace,
const char *psz_mode );
LIBVLC_API int libvlc_video_get_marquee_int( libvlc_media_player_t *p_mi,
unsigned option );
LIBVLC_API void libvlc_video_set_marquee_int( libvlc_media_player_t *p_mi,
unsigned option, int i_val );
LIBVLC_API void libvlc_video_set_marquee_string( libvlc_media_player_t *p_mi,
unsigned option, const char *psz_text );
enum libvlc_video_logo_option_t {
libvlc_logo_enable,
libvlc_logo_file, 
libvlc_logo_x,
libvlc_logo_y,
libvlc_logo_delay,
libvlc_logo_repeat,
libvlc_logo_opacity,
libvlc_logo_position
};
LIBVLC_API int libvlc_video_get_logo_int( libvlc_media_player_t *p_mi,
unsigned option );
LIBVLC_API void libvlc_video_set_logo_int( libvlc_media_player_t *p_mi,
unsigned option, int value );
LIBVLC_API void libvlc_video_set_logo_string( libvlc_media_player_t *p_mi,
unsigned option, const char *psz_value );
enum libvlc_video_adjust_option_t {
libvlc_adjust_Enable = 0,
libvlc_adjust_Contrast,
libvlc_adjust_Brightness,
libvlc_adjust_Hue,
libvlc_adjust_Saturation,
libvlc_adjust_Gamma
};
LIBVLC_API int libvlc_video_get_adjust_int( libvlc_media_player_t *p_mi,
unsigned option );
LIBVLC_API void libvlc_video_set_adjust_int( libvlc_media_player_t *p_mi,
unsigned option, int value );
LIBVLC_API float libvlc_video_get_adjust_float( libvlc_media_player_t *p_mi,
unsigned option );
LIBVLC_API void libvlc_video_set_adjust_float( libvlc_media_player_t *p_mi,
unsigned option, float value );
typedef enum libvlc_audio_output_channel_t {
libvlc_AudioChannel_Error = -1,
libvlc_AudioChannel_Stereo = 1,
libvlc_AudioChannel_RStereo = 2,
libvlc_AudioChannel_Left = 3,
libvlc_AudioChannel_Right = 4,
libvlc_AudioChannel_Dolbys = 5
} libvlc_audio_output_channel_t;
LIBVLC_API libvlc_audio_output_t *
libvlc_audio_output_list_get( libvlc_instance_t *p_instance );
LIBVLC_API
void libvlc_audio_output_list_release( libvlc_audio_output_t *p_list );
LIBVLC_API int libvlc_audio_output_set( libvlc_media_player_t *p_mi,
const char *psz_name );
LIBVLC_API libvlc_audio_output_device_t *
libvlc_audio_output_device_enum( libvlc_media_player_t *mp );
LIBVLC_API libvlc_audio_output_device_t *
libvlc_audio_output_device_list_get( libvlc_instance_t *p_instance,
const char *aout );
LIBVLC_API void libvlc_audio_output_device_list_release(
libvlc_audio_output_device_t *p_list );
LIBVLC_API void libvlc_audio_output_device_set( libvlc_media_player_t *mp,
const char *module,
const char *device_id );
LIBVLC_API char *libvlc_audio_output_device_get( libvlc_media_player_t *mp );
LIBVLC_API void libvlc_audio_toggle_mute( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_get_mute( libvlc_media_player_t *p_mi );
LIBVLC_API void libvlc_audio_set_mute( libvlc_media_player_t *p_mi, int status );
LIBVLC_API int libvlc_audio_get_volume( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_set_volume( libvlc_media_player_t *p_mi, int i_volume );
LIBVLC_API int libvlc_audio_get_track_count( libvlc_media_player_t *p_mi );
LIBVLC_API libvlc_track_description_t *
libvlc_audio_get_track_description( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_get_track( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_set_track( libvlc_media_player_t *p_mi, int i_track );
LIBVLC_API int libvlc_audio_get_channel( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_set_channel( libvlc_media_player_t *p_mi, int channel );
LIBVLC_API int64_t libvlc_audio_get_delay( libvlc_media_player_t *p_mi );
LIBVLC_API int libvlc_audio_set_delay( libvlc_media_player_t *p_mi, int64_t i_delay );
LIBVLC_API unsigned libvlc_audio_equalizer_get_preset_count( void );
LIBVLC_API const char *libvlc_audio_equalizer_get_preset_name( unsigned u_index );
LIBVLC_API unsigned libvlc_audio_equalizer_get_band_count( void );
LIBVLC_API float libvlc_audio_equalizer_get_band_frequency( unsigned u_index );
LIBVLC_API libvlc_equalizer_t *libvlc_audio_equalizer_new( void );
LIBVLC_API libvlc_equalizer_t *libvlc_audio_equalizer_new_from_preset( unsigned u_index );
LIBVLC_API void libvlc_audio_equalizer_release( libvlc_equalizer_t *p_equalizer );
LIBVLC_API int libvlc_audio_equalizer_set_preamp( libvlc_equalizer_t *p_equalizer, float f_preamp );
LIBVLC_API float libvlc_audio_equalizer_get_preamp( libvlc_equalizer_t *p_equalizer );
LIBVLC_API int libvlc_audio_equalizer_set_amp_at_index( libvlc_equalizer_t *p_equalizer, float f_amp, unsigned u_band );
LIBVLC_API float libvlc_audio_equalizer_get_amp_at_index( libvlc_equalizer_t *p_equalizer, unsigned u_band );
LIBVLC_API int libvlc_media_player_set_equalizer( libvlc_media_player_t *p_mi, libvlc_equalizer_t *p_equalizer );
typedef enum libvlc_media_player_role {
libvlc_role_None = 0, 
libvlc_role_Music, 
libvlc_role_Video, 
libvlc_role_Communication, 
libvlc_role_Game, 
libvlc_role_Notification, 
libvlc_role_Animation, 
libvlc_role_Production, 
libvlc_role_Accessibility, 
libvlc_role_Test 
#define libvlc_role_Last libvlc_role_Test
} libvlc_media_player_role_t;
LIBVLC_API int libvlc_media_player_get_role(libvlc_media_player_t *p_mi);
LIBVLC_API int libvlc_media_player_set_role(libvlc_media_player_t *p_mi,
unsigned role);
#if defined(__cplusplus)
}
#endif
