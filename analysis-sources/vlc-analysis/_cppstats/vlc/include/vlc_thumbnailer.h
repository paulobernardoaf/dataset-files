#include <vlc_common.h>
typedef struct vlc_thumbnailer_t vlc_thumbnailer_t;
typedef struct vlc_thumbnailer_request_t vlc_thumbnailer_request_t;
typedef void(*vlc_thumbnailer_cb)( void* data, picture_t* thumbnail );
VLC_API vlc_thumbnailer_t*
vlc_thumbnailer_Create( vlc_object_t* p_parent )
VLC_USED;
enum vlc_thumbnailer_seek_speed
{
VLC_THUMBNAILER_SEEK_PRECISE,
VLC_THUMBNAILER_SEEK_FAST,
};
VLC_API vlc_thumbnailer_request_t*
vlc_thumbnailer_RequestByTime( vlc_thumbnailer_t *thumbnailer,
vlc_tick_t time,
enum vlc_thumbnailer_seek_speed speed,
input_item_t *input_item, vlc_tick_t timeout,
vlc_thumbnailer_cb cb, void* user_data );
VLC_API vlc_thumbnailer_request_t*
vlc_thumbnailer_RequestByPos( vlc_thumbnailer_t *thumbnailer,
float pos,
enum vlc_thumbnailer_seek_speed speed,
input_item_t *input_item, vlc_tick_t timeout,
vlc_thumbnailer_cb cb, void* user_data );
VLC_API void
vlc_thumbnailer_Cancel( vlc_thumbnailer_t* thumbnailer,
vlc_thumbnailer_request_t* request );
VLC_API void vlc_thumbnailer_Release( vlc_thumbnailer_t* thumbnailer );
