#include <vlc_es.h>
#include <vlc_picture.h>
#include <vlc_codec.h>
typedef struct vlc_video_context vlc_video_context;
struct filter_video_callbacks
{
picture_t *(*buffer_new)(filter_t *);
vlc_decoder_device * (*hold_device)(vlc_object_t *, void *sys);
};
struct filter_subpicture_callbacks
{
subpicture_t *(*buffer_new)(filter_t *);
};
typedef struct filter_owner_t
{
union
{
const struct filter_video_callbacks *video;
const struct filter_subpicture_callbacks *sub;
};
void *sys;
} filter_owner_t;
struct vlc_mouse_t;
struct filter_t
{
struct vlc_object_t obj;
module_t * p_module;
void *p_sys;
es_format_t fmt_in;
vlc_video_context *vctx_in; 
es_format_t fmt_out;
vlc_video_context *vctx_out; 
bool b_allow_fmt_out_change;
const char * psz_name;
config_chain_t * p_cfg;
union
{
picture_t * (*pf_video_filter)( filter_t *, picture_t * );
block_t * (*pf_audio_filter)( filter_t *, block_t * );
void (*pf_video_blend)( filter_t *, picture_t *, const picture_t *,
int, int, int );
subpicture_t *(*pf_sub_source)( filter_t *, vlc_tick_t );
subpicture_t *(*pf_sub_filter)( filter_t *, subpicture_t * );
int (*pf_render)( filter_t *, subpicture_region_t *,
subpicture_region_t *, const vlc_fourcc_t * );
};
union
{
block_t *(*pf_audio_drain) ( filter_t * );
};
void (*pf_flush)( filter_t * );
void (*pf_change_viewpoint)( filter_t *, const vlc_viewpoint_t * );
union
{
int (*pf_video_mouse)( filter_t *, struct vlc_mouse_t *,
const struct vlc_mouse_t *p_old,
const struct vlc_mouse_t *p_new );
};
int (*pf_get_attachments)( filter_t *, input_attachment_t ***, int * );
filter_owner_t owner;
};
static inline picture_t *filter_NewPicture( filter_t *p_filter )
{
picture_t *pic = NULL;
if ( p_filter->owner.video != NULL && p_filter->owner.video->buffer_new != NULL)
pic = p_filter->owner.video->buffer_new( p_filter );
if ( pic == NULL )
{
pic = picture_NewFromFormat( &p_filter->fmt_out.video );
}
if( pic == NULL )
msg_Warn( p_filter, "can't get output picture" );
return pic;
}
static inline void filter_Flush( filter_t *p_filter )
{
if( p_filter->pf_flush != NULL )
p_filter->pf_flush( p_filter );
}
static inline void filter_ChangeViewpoint( filter_t *p_filter,
const vlc_viewpoint_t *vp)
{
if( p_filter->pf_change_viewpoint != NULL )
p_filter->pf_change_viewpoint( p_filter, vp );
}
static inline vlc_decoder_device * filter_HoldDecoderDevice( filter_t *p_filter )
{
if ( !p_filter->owner.video || !p_filter->owner.video->hold_device )
return NULL;
return p_filter->owner.video->hold_device( VLC_OBJECT(p_filter), p_filter->owner.sys );
}
static inline vlc_decoder_device * filter_HoldDecoderDeviceType( filter_t *p_filter,
enum vlc_decoder_device_type type )
{
if ( !p_filter->owner.video || !p_filter->owner.video->hold_device )
return NULL;
vlc_decoder_device *dec_dev = p_filter->owner.video->hold_device( VLC_OBJECT(p_filter),
p_filter->owner.sys );
if ( dec_dev != NULL )
{
if ( dec_dev->type == type )
return dec_dev;
vlc_decoder_device_Release(dec_dev);
}
return NULL;
}
static inline block_t *filter_DrainAudio( filter_t *p_filter )
{
if( p_filter->pf_audio_drain )
return p_filter->pf_audio_drain( p_filter );
else
return NULL;
}
static inline subpicture_t *filter_NewSubpicture( filter_t *p_filter )
{
subpicture_t *subpic = p_filter->owner.sub->buffer_new( p_filter );
if( subpic == NULL )
msg_Warn( p_filter, "can't get output subpicture" );
return subpic;
}
static inline int filter_GetInputAttachments( filter_t *p_filter,
input_attachment_t ***ppp_attachment,
int *pi_attachment )
{
if( !p_filter->pf_get_attachments )
return VLC_EGENERIC;
return p_filter->pf_get_attachments( p_filter,
ppp_attachment, pi_attachment );
}
VLC_API void filter_AddProxyCallbacks( vlc_object_t *obj, filter_t *filter,
vlc_callback_t restart_cb );
#define filter_AddProxyCallbacks(a, b, c) filter_AddProxyCallbacks(VLC_OBJECT(a), b, c)
VLC_API void filter_DelProxyCallbacks( vlc_object_t *obj, filter_t *filter,
vlc_callback_t restart_cb);
#define filter_DelProxyCallbacks(a, b, c) filter_DelProxyCallbacks(VLC_OBJECT(a), b, c)
typedef filter_t vlc_blender_t;
VLC_API vlc_blender_t * filter_NewBlend( vlc_object_t *, const video_format_t *p_dst_chroma ) VLC_USED;
VLC_API int filter_ConfigureBlend( vlc_blender_t *, int i_dst_width, int i_dst_height, const video_format_t *p_src );
VLC_API int filter_Blend( vlc_blender_t *, picture_t *p_dst, int i_dst_x, int i_dst_y, const picture_t *p_src, int i_alpha );
VLC_API void filter_DeleteBlend( vlc_blender_t * );
#define VIDEO_FILTER_WRAPPER( name ) static picture_t *name ##_Filter ( filter_t *p_filter, picture_t *p_pic ) { picture_t *p_outpic = filter_NewPicture( p_filter ); if( p_outpic ) { name( p_filter, p_pic, p_outpic ); picture_CopyProperties( p_outpic, p_pic ); } picture_Release( p_pic ); return p_outpic; }
typedef struct filter_chain_t filter_chain_t;
filter_chain_t * filter_chain_NewSPU( vlc_object_t *obj, const char *psz_capability )
VLC_USED;
#define filter_chain_NewSPU( a, b ) filter_chain_NewSPU( VLC_OBJECT( a ), b )
VLC_API filter_chain_t * filter_chain_NewVideo( vlc_object_t *obj, bool change,
const filter_owner_t *owner )
VLC_USED;
#define filter_chain_NewVideo( a, b, c ) filter_chain_NewVideo( VLC_OBJECT( a ), b, c )
VLC_API void filter_chain_Delete( filter_chain_t * );
VLC_API void filter_chain_Reset( filter_chain_t *p_chain,
const es_format_t *p_fmt_in,
vlc_video_context *vctx_in,
const es_format_t *p_fmt_out );
VLC_API void filter_chain_Clear(filter_chain_t *);
VLC_API filter_t *filter_chain_AppendFilter(filter_chain_t *chain,
const char *name, config_chain_t *cfg,
const es_format_t *fmt_out);
VLC_API int filter_chain_AppendConverter(filter_chain_t *chain,
const es_format_t *fmt_out);
VLC_API int filter_chain_AppendFromString(filter_chain_t *chain,
const char *str);
VLC_API void filter_chain_DeleteFilter(filter_chain_t *chain,
filter_t *filter);
VLC_API bool filter_chain_IsEmpty(const filter_chain_t *chain);
VLC_API const es_format_t *filter_chain_GetFmtOut(const filter_chain_t *chain);
VLC_API vlc_video_context *filter_chain_GetVideoCtxOut(const filter_chain_t *chain);
VLC_API picture_t *filter_chain_VideoFilter(filter_chain_t *chain,
picture_t *pic);
VLC_API void filter_chain_VideoFlush( filter_chain_t * );
void filter_chain_SubSource(filter_chain_t *chain, spu_t *,
vlc_tick_t display_date);
VLC_API subpicture_t *filter_chain_SubFilter(filter_chain_t *chain,
subpicture_t *subpic);
VLC_API int filter_chain_MouseFilter( filter_chain_t *, struct vlc_mouse_t *,
const struct vlc_mouse_t * );
int filter_chain_ForEach( filter_chain_t *chain,
int (*cb)( filter_t *, void * ), void *opaque );
