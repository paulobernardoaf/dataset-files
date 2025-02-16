#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#define SRC_FOURCC "I422,J422"
#define DEST_FOURCC "I420,IYUV,J420,YV12,YUVA"
static int Activate ( vlc_object_t * );
static void I422_I420( filter_t *, picture_t *, picture_t * );
static void I422_YV12( filter_t *, picture_t *, picture_t * );
static void I422_YUVA( filter_t *, picture_t *, picture_t * );
static picture_t *I422_I420_Filter( filter_t *, picture_t * );
static picture_t *I422_YV12_Filter( filter_t *, picture_t * );
static picture_t *I422_YUVA_Filter( filter_t *, picture_t * );
vlc_module_begin ()
set_description( N_("Conversions from " SRC_FOURCC " to " DEST_FOURCC) )
set_capability( "video converter", 60 )
set_callback( Activate )
vlc_module_end ()
static int Activate( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
if( p_filter->fmt_in.video.i_width & 1
|| p_filter->fmt_in.video.i_height & 1 )
{
return -1;
}
if( p_filter->fmt_in.video.i_width != p_filter->fmt_out.video.i_width
|| p_filter->fmt_in.video.i_height != p_filter->fmt_out.video.i_height
|| p_filter->fmt_in.video.orientation != p_filter->fmt_out.video.orientation )
return -1;
switch( p_filter->fmt_in.video.i_chroma )
{
case VLC_CODEC_I422:
case VLC_CODEC_J422:
switch( p_filter->fmt_out.video.i_chroma )
{
case VLC_CODEC_I420:
case VLC_CODEC_J420:
p_filter->pf_video_filter = I422_I420_Filter;
break;
case VLC_CODEC_YV12:
p_filter->pf_video_filter = I422_YV12_Filter;
break;
case VLC_CODEC_YUV420A:
p_filter->pf_video_filter = I422_YUVA_Filter;
break;
default:
return -1;
}
break;
default:
return -1;
}
return 0;
}
VIDEO_FILTER_WRAPPER( I422_I420 )
VIDEO_FILTER_WRAPPER( I422_YV12 )
VIDEO_FILTER_WRAPPER( I422_YUVA )
static void I422_I420( filter_t *p_filter, picture_t *p_source,
picture_t *p_dest )
{
uint16_t i_dpy = p_dest->p[Y_PLANE].i_pitch;
uint16_t i_spy = p_source->p[Y_PLANE].i_pitch;
uint16_t i_dpuv = p_dest->p[U_PLANE].i_pitch;
uint16_t i_spuv = p_source->p[U_PLANE].i_pitch;
uint16_t i_width = p_filter->fmt_in.video.i_width;
uint16_t i_y = p_filter->fmt_in.video.i_height;
uint8_t *p_dy = p_dest->Y_PIXELS + (i_y-1)*i_dpy;
uint8_t *p_y = p_source->Y_PIXELS + (i_y-1)*i_spy;
uint8_t *p_du = p_dest->U_PIXELS + (i_y/2-1)*i_dpuv;
uint8_t *p_u = p_source->U_PIXELS + (i_y-1)*i_spuv;
uint8_t *p_dv = p_dest->V_PIXELS + (i_y/2-1)*i_dpuv;
uint8_t *p_v = p_source->V_PIXELS + (i_y-1)*i_spuv;
i_y /= 2;
for ( ; i_y--; )
{
memcpy(p_dy, p_y, i_width); p_dy -= i_dpy; p_y -= i_spy;
memcpy(p_dy, p_y, i_width); p_dy -= i_dpy; p_y -= i_spy;
memcpy(p_du, p_u, i_width/2); p_du -= i_dpuv; p_u -= 2*i_spuv;
memcpy(p_dv, p_v, i_width/2); p_dv -= i_dpuv; p_v -= 2*i_spuv;
}
}
static void I422_YV12( filter_t *p_filter, picture_t *p_source,
picture_t *p_dest )
{
uint16_t i_dpy = p_dest->p[Y_PLANE].i_pitch;
uint16_t i_spy = p_source->p[Y_PLANE].i_pitch;
uint16_t i_dpuv = p_dest->p[U_PLANE].i_pitch;
uint16_t i_spuv = p_source->p[U_PLANE].i_pitch;
uint16_t i_width = p_filter->fmt_in.video.i_width;
uint16_t i_y = p_filter->fmt_in.video.i_height;
uint8_t *p_dy = p_dest->Y_PIXELS + (i_y-1)*i_dpy;
uint8_t *p_y = p_source->Y_PIXELS + (i_y-1)*i_spy;
uint8_t *p_du = p_dest->V_PIXELS + (i_y/2-1)*i_dpuv; 
uint8_t *p_u = p_source->U_PIXELS + (i_y-1)*i_spuv;
uint8_t *p_dv = p_dest->U_PIXELS + (i_y/2-1)*i_dpuv; 
uint8_t *p_v = p_source->V_PIXELS + (i_y-1)*i_spuv;
i_y /= 2;
for ( ; i_y--; )
{
memcpy(p_dy, p_y, i_width); p_dy -= i_dpy; p_y -= i_spy;
memcpy(p_dy, p_y, i_width); p_dy -= i_dpy; p_y -= i_spy;
memcpy(p_du, p_u, i_width/2); p_du -= i_dpuv; p_u -= 2*i_spuv;
memcpy(p_dv, p_v, i_width/2); p_dv -= i_dpuv; p_v -= 2*i_spuv;
}
}
static void I422_YUVA( filter_t *p_filter, picture_t *p_source,
picture_t *p_dest )
{
I422_I420( p_filter, p_source, p_dest );
memset( p_dest->p[A_PLANE].p_pixels, 0xff,
p_dest->p[A_PLANE].i_lines * p_dest->p[A_PLANE].i_pitch );
}
