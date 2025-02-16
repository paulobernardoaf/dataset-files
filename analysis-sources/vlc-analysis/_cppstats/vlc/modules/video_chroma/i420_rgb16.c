#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_cpu.h>
#include "i420_rgb.h"
#include "i420_rgb_c.h"
static void SetOffset( int i_width, int i_height, int i_pic_width,
int i_pic_height, bool *pb_hscale,
unsigned int *pi_vscale, int *p_offset )
{
if( i_pic_width - i_width == 0 )
{ 
*pb_hscale = 0;
}
else if( i_pic_width - i_width > 0 )
{ 
int i_scale_count = i_pic_width;
*pb_hscale = 1;
for( int i_x = i_width; i_x--; )
{
while( (i_scale_count -= i_width) > 0 )
{
*p_offset++ = 0;
}
*p_offset++ = 1;
i_scale_count += i_pic_width;
}
}
else 
{ 
int i_scale_count = i_pic_width;
*pb_hscale = 1;
for( int i_x = i_pic_width; i_x--; )
{
*p_offset = 1;
while( (i_scale_count -= i_pic_width) > 0 )
{
*p_offset += 1;
}
p_offset++;
i_scale_count += i_width;
}
}
if( i_pic_height - i_height == 0 )
*pi_vscale = 0;
else if( i_pic_height - i_height > 0 )
*pi_vscale = 1;
else 
*pi_vscale = -1;
}
void I420_RGB16( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;
uint16_t *p_pic = (uint16_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;
bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 
int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint16_t * p_pic_start; 
int i_uval, i_vval; 
int i_red, i_green, i_blue; 
uint16_t * p_yuv = p_sys->p_rgb16;
uint16_t * p_ybase; 
uint16_t * p_buffer_start;
uint16_t * p_buffer;
int * p_offset_start = p_sys->p_offset;
int * p_offset;
const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );
i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;
i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;
SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );
if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint16_t*)p_sys->p_buffer;
i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);
for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;
for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
}
if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
CONVERT_YUV_PIXEL(2); CONVERT_Y_PIXEL(2);
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 2 );
p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}
}
void I420_RGB32( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;
uint32_t *p_pic = (uint32_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;
bool b_hscale; 
unsigned int i_vscale; 
unsigned int i_x, i_y; 
int i_right_margin;
int i_rewind;
int i_scale_count; 
int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2; 
uint32_t * p_pic_start; 
int i_uval, i_vval; 
int i_red, i_green, i_blue; 
uint32_t * p_yuv = p_sys->p_rgb32;
uint32_t * p_ybase; 
uint32_t * p_buffer_start;
uint32_t * p_buffer;
int * p_offset_start = p_sys->p_offset;
int * p_offset;
const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );
i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;
i_rewind = (-(p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width)) & 7;
SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );
if(b_hscale &&
AllocateOrGrow(&p_sys->p_buffer, &p_sys->i_buffer_size,
p_filter->fmt_in.video.i_x_offset +
p_filter->fmt_in.video.i_visible_width,
p_sys->i_bytespp))
return;
else p_buffer_start = (uint32_t*)p_sys->p_buffer;
i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);
for( i_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
p_pic_start = p_pic;
p_buffer = b_hscale ? p_buffer_start : p_pic;
for ( i_x = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 8; i_x--; )
{
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
}
if( i_rewind )
{
p_y -= i_rewind;
p_u -= i_rewind >> 1;
p_v -= i_rewind >> 1;
p_buffer -= i_rewind;
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
CONVERT_YUV_PIXEL(4); CONVERT_Y_PIXEL(4);
}
SCALE_WIDTH;
SCALE_HEIGHT( 420, 4 );
p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}
}
