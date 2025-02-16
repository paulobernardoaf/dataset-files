#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#define SRC_FOURCC "GREY"
#define DEST_FOURCC "I420,YUY2"
static int Activate ( vlc_object_t * );
static void GREY_I420( filter_t *, picture_t *, picture_t * );
static void GREY_YUY2( filter_t *, picture_t *, picture_t * );
static picture_t *GREY_I420_Filter( filter_t *, picture_t * );
static picture_t *GREY_YUY2_Filter( filter_t *, picture_t * );
vlc_module_begin ()
set_description( N_("Conversions from " SRC_FOURCC " to " DEST_FOURCC) )
set_capability( "video converter", 80 )
set_callback( Activate )
vlc_module_end ()
static int Activate( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
if( p_filter->fmt_out.video.i_width & 1
|| p_filter->fmt_out.video.i_height & 1 )
{
return -1;
}
if( p_filter->fmt_in.video.i_width != p_filter->fmt_out.video.i_width
|| p_filter->fmt_in.video.i_height != p_filter->fmt_out.video.i_height
|| p_filter->fmt_in.video.orientation != p_filter->fmt_out.video.orientation )
return -1;
switch( p_filter->fmt_in.video.i_chroma )
{
case VLC_CODEC_GREY:
switch( p_filter->fmt_out.video.i_chroma )
{
case VLC_CODEC_I420:
p_filter->pf_video_filter = GREY_I420_Filter;
break;
case VLC_CODEC_YUYV:
p_filter->pf_video_filter = GREY_YUY2_Filter;
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
VIDEO_FILTER_WRAPPER( GREY_I420 )
VIDEO_FILTER_WRAPPER( GREY_YUY2 )
static void GREY_I420( filter_t *p_filter, picture_t *p_source,
picture_t *p_dest )
{
uint8_t *p_line = p_source->p->p_pixels;
uint8_t *p_y = p_dest->Y_PIXELS;
uint8_t *p_u = p_dest->U_PIXELS;
uint8_t *p_v = p_dest->V_PIXELS;
int i_x, i_y;
const int i_source_margin = p_source->p->i_pitch
- p_source->p->i_visible_pitch;
const int i_dest_margin = p_dest->p[0].i_pitch
- p_dest->p[0].i_visible_pitch;
const int i_dest_margin_c = p_dest->p[1].i_pitch
- p_dest->p[1].i_visible_pitch;
for( i_y = p_filter->fmt_in.video.i_height / 2; i_y-- ; )
{
memset(p_u, 0x80, p_dest->p[1].i_visible_pitch);
p_u += i_dest_margin_c;
memset(p_v, 0x80, p_dest->p[1].i_visible_pitch);
p_v += i_dest_margin_c;
}
for( i_y = p_filter->fmt_in.video.i_height; i_y-- ; )
{
for( i_x = p_filter->fmt_in.video.i_width / 8; i_x-- ; )
{
*p_y++ = *p_line++; *p_y++ = *p_line++;
*p_y++ = *p_line++; *p_y++ = *p_line++;
*p_y++ = *p_line++; *p_y++ = *p_line++;
*p_y++ = *p_line++; *p_y++ = *p_line++;
}
for( i_x = p_filter->fmt_in.video.i_width % 8; i_x-- ; )
{
*p_y++ = *p_line++;
}
p_line += i_source_margin;
p_y += i_dest_margin;
}
}
static void GREY_YUY2( filter_t *p_filter, picture_t *p_source,
picture_t *p_dest )
{
uint8_t *p_in = p_source->p->p_pixels;
uint8_t *p_out = p_dest->p->p_pixels;
int i_x, i_y;
const int i_source_margin = p_source->p->i_pitch
- p_source->p->i_visible_pitch;
const int i_dest_margin = p_dest->p->i_pitch
- p_dest->p->i_visible_pitch;
for( i_y = p_filter->fmt_out.video.i_height; i_y-- ; )
{
for( i_x = p_filter->fmt_out.video.i_width / 8; i_x-- ; )
{
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
}
for( i_x = (p_filter->fmt_out.video.i_width % 8) / 2; i_x-- ; )
{
*p_out++ = *p_in++; *p_out++ = 0x80;
*p_out++ = *p_in++; *p_out++ = 0x80;
}
p_in += i_source_margin;
p_out += i_dest_margin;
}
}
