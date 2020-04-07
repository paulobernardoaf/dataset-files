#include <stdlib.h>
#include <string.h>
#include <vlc_es.h>
#include <vlc_stream.h>
#include <vlc_es_out.h>
#define VLC_DEMUXER_EOF 0
#define VLC_DEMUXER_EGENERIC -1
#define VLC_DEMUXER_SUCCESS 1
#define INPUT_UPDATE_TITLE 0x0010
#define INPUT_UPDATE_SEEKPOINT 0x0020
#define INPUT_UPDATE_META 0x0040
#define INPUT_UPDATE_TITLE_LIST 0x0100
typedef struct demux_meta_t
{
struct vlc_object_t obj;
input_item_t *p_item; 
vlc_meta_t *p_meta; 
int i_attachments; 
input_attachment_t **attachments; 
} demux_meta_t;
enum demux_query_e
{
DEMUX_CAN_SEEK,
DEMUX_CAN_PAUSE = 0x002,
DEMUX_CAN_CONTROL_PACE,
DEMUX_GET_PTS_DELAY = 0x101,
DEMUX_GET_META = 0x105,
DEMUX_GET_SIGNAL = 0x107,
DEMUX_SET_PAUSE_STATE = 0x200,
DEMUX_SET_TITLE,
DEMUX_SET_SEEKPOINT, 
DEMUX_TEST_AND_CLEAR_FLAGS, 
DEMUX_GET_TITLE, 
DEMUX_GET_SEEKPOINT, 
DEMUX_GET_POSITION = 0x300, 
DEMUX_SET_POSITION, 
DEMUX_GET_LENGTH, 
DEMUX_GET_TIME, 
DEMUX_SET_TIME, 
DEMUX_GET_NORMAL_TIME, 
DEMUX_GET_TITLE_INFO,
DEMUX_SET_GROUP_DEFAULT,
DEMUX_SET_GROUP_ALL,
DEMUX_SET_GROUP_LIST, 
DEMUX_SET_ES, 
DEMUX_SET_ES_LIST, 
DEMUX_SET_NEXT_DEMUX_TIME, 
DEMUX_GET_FPS, 
DEMUX_HAS_UNSUPPORTED_META, 
DEMUX_GET_ATTACHMENTS, 
DEMUX_CAN_RECORD, 
DEMUX_SET_RECORD_STATE,
DEMUX_CAN_CONTROL_RATE, 
DEMUX_SET_RATE, 
DEMUX_IS_PLAYLIST,
DEMUX_NAV_ACTIVATE,
DEMUX_NAV_UP,
DEMUX_NAV_DOWN,
DEMUX_NAV_LEFT,
DEMUX_NAV_RIGHT,
DEMUX_NAV_POPUP,
DEMUX_NAV_MENU, 
DEMUX_FILTER_ENABLE,
DEMUX_FILTER_DISABLE
};
VLC_API demux_t *demux_New( vlc_object_t *p_obj, const char *psz_name,
stream_t *s, es_out_t *out );
static inline void demux_Delete(demux_t *demux)
{
vlc_stream_Delete(demux);
}
VLC_API int demux_vaControlHelper( stream_t *, int64_t i_start, int64_t i_end,
int64_t i_bitrate, int i_align, int i_query, va_list args );
VLC_USED static inline int demux_Demux( demux_t *p_demux )
{
if( !p_demux->pf_demux )
return VLC_DEMUXER_SUCCESS;
return p_demux->pf_demux( p_demux );
}
VLC_API int demux_vaControl( demux_t *p_demux, int i_query, va_list args );
static inline int demux_Control( demux_t *p_demux, int i_query, ... )
{
va_list args;
int i_result;
va_start( args, i_query );
i_result = demux_vaControl( p_demux, i_query, args );
va_end( args );
return i_result;
}
#if !defined(__cplusplus)
static inline void demux_UpdateTitleFromStream( demux_t *demux,
int *restrict titlep, int *restrict seekpointp,
unsigned *restrict updatep )
{
stream_t *s = demux->s;
unsigned title, seekpoint;
if( vlc_stream_Control( s, STREAM_GET_TITLE, &title ) == VLC_SUCCESS
&& title != (unsigned)*titlep )
{
*titlep = title;
*updatep |= INPUT_UPDATE_TITLE;
}
if( vlc_stream_Control( s, STREAM_GET_SEEKPOINT,
&seekpoint ) == VLC_SUCCESS
&& seekpoint != (unsigned)*seekpointp )
{
*seekpointp = seekpoint;
*updatep |= INPUT_UPDATE_SEEKPOINT;
}
}
#define demux_UpdateTitleFromStream(demux) demux_UpdateTitleFromStream(demux, &((demux_sys_t *)((demux)->p_sys))->current_title, &((demux_sys_t *)((demux)->p_sys))->current_seekpoint, &((demux_sys_t *)((demux)->p_sys))->updates)
#endif
VLC_USED
static inline bool demux_IsPathExtension( demux_t *p_demux, const char *psz_extension )
{
const char *name = (p_demux->psz_filepath != NULL) ? p_demux->psz_filepath
: p_demux->psz_location;
const char *psz_ext = strrchr ( name, '.' );
if( !psz_ext || strcasecmp( psz_ext, psz_extension ) )
return false;
return true;
}
VLC_USED
static inline bool demux_IsContentType(demux_t *demux, const char *type)
{
return stream_IsMimeType(demux->s, type);
}
VLC_USED
static inline bool demux_IsForced( demux_t *p_demux, const char *psz_name )
{
if( p_demux->psz_name == NULL || strcmp( p_demux->psz_name, psz_name ) )
return false;
return true;
}
static inline int demux_SetPosition( demux_t *p_demux, double pos, bool precise,
bool absolute)
{
if( !absolute )
{
double current_pos;
int ret = demux_Control( p_demux, DEMUX_GET_POSITION, &current_pos );
if( ret != VLC_SUCCESS )
return ret;
pos += current_pos;
}
if( pos < 0.f )
pos = 0.f;
else if( pos > 1.f )
pos = 1.f;
return demux_Control( p_demux, DEMUX_SET_POSITION, pos, precise );
}
static inline int demux_SetTime( demux_t *p_demux, vlc_tick_t time, bool precise,
bool absolute )
{
if( !absolute )
{
vlc_tick_t current_time;
int ret = demux_Control( p_demux, DEMUX_GET_TIME, &current_time );
if( ret != VLC_SUCCESS )
return ret;
time += current_time;
}
if( time < 0 )
time = 0;
return demux_Control( p_demux, DEMUX_SET_TIME, time, precise );
}
VLC_API decoder_t * demux_PacketizerNew( demux_t *p_demux, es_format_t *p_fmt, const char *psz_msg ) VLC_USED;
VLC_API void demux_PacketizerDestroy( decoder_t *p_packetizer );
#define DEMUX_INIT_COMMON() do { p_demux->pf_control = Control; p_demux->pf_demux = Demux; p_demux->p_sys = calloc( 1, sizeof( demux_sys_t ) ); if( !p_demux->p_sys ) return VLC_ENOMEM;} while(0)
typedef struct vlc_demux_chained_t vlc_demux_chained_t;
VLC_API vlc_demux_chained_t *vlc_demux_chained_New(vlc_object_t *parent,
const char *name,
es_out_t *out);
VLC_API void vlc_demux_chained_Delete(vlc_demux_chained_t *);
VLC_API void vlc_demux_chained_Send(vlc_demux_chained_t *, block_t *block);
VLC_API int vlc_demux_chained_ControlVa(vlc_demux_chained_t *, int query,
va_list args);
static inline int vlc_demux_chained_Control(vlc_demux_chained_t *dc, int query,
...)
{
va_list ap;
int ret;
va_start(ap, query);
ret = vlc_demux_chained_ControlVa(dc, query, ap);
va_end(ap);
return ret;
}
