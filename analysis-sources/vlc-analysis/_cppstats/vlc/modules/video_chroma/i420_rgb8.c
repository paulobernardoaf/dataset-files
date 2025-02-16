#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "i420_rgb.h"
#include "i420_rgb_c.h"
static void SetOffset( int, int, int, int, bool *, int *, int * );
void I420_RGB8( filter_t *p_filter, picture_t *p_src, picture_t *p_dest )
{
filter_sys_t *p_sys = p_filter->p_sys;
uint8_t *p_pic = (uint8_t*)p_dest->p->p_pixels;
uint8_t *p_y = p_src->Y_PIXELS;
uint8_t *p_u = p_src->U_PIXELS;
uint8_t *p_v = p_src->V_PIXELS;
bool b_hscale; 
int i_vscale; 
unsigned int i_x, i_y; 
unsigned int i_real_y; 
int i_right_margin;
int i_scale_count; 
unsigned int i_chroma_width = (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width) / 2;
uint8_t * p_lookup = p_sys->p_base;
int * p_offset_start = p_sys->p_offset;
int * p_offset;
const int i_source_margin = p_src->p[0].i_pitch
- p_src->p[0].i_visible_pitch
- p_filter->fmt_in.video.i_x_offset;
const int i_source_margin_c = p_src->p[1].i_pitch
- p_src->p[1].i_visible_pitch
- ( p_filter->fmt_in.video.i_x_offset / 2 );
static const int dither10[4] = { 0x0, 0x8, 0x2, 0xa };
static const int dither11[4] = { 0xc, 0x4, 0xe, 0x6 };
static const int dither12[4] = { 0x3, 0xb, 0x1, 0x9 };
static const int dither13[4] = { 0xf, 0x7, 0xd, 0x5 };
static const int dither20[4] = { 0x0, 0x10, 0x4, 0x14 };
static const int dither21[4] = { 0x18, 0x8, 0x1c, 0xc };
static const int dither22[4] = { 0x6, 0x16, 0x2, 0x12 };
static const int dither23[4] = { 0x1e, 0xe, 0x1a, 0xa };
SetOffset( (p_filter->fmt_in.video.i_x_offset + p_filter->fmt_in.video.i_visible_width),
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height),
(p_filter->fmt_out.video.i_x_offset + p_filter->fmt_out.video.i_visible_width),
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height),
&b_hscale, &i_vscale, p_offset_start );
i_right_margin = p_dest->p->i_pitch - p_dest->p->i_visible_pitch;
i_scale_count = ( i_vscale == 1 ) ?
(p_filter->fmt_out.video.i_y_offset + p_filter->fmt_out.video.i_visible_height) :
(p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height);
for( i_y = 0, i_real_y = 0; i_y < (p_filter->fmt_in.video.i_y_offset + p_filter->fmt_in.video.i_visible_height); i_y++ )
{
SCALE_WIDTH_DITHER( 420 );
SCALE_HEIGHT_DITHER( 420 );
}
p_y += i_source_margin;
if( i_y % 2 )
{
p_u += i_source_margin_c;
p_v += i_source_margin_c;
}
}
static void SetOffset( int i_width, int i_height, int i_pic_width,
int i_pic_height, bool *pb_hscale,
int *pi_vscale, int *p_offset )
{
int i_x; 
int i_scale_count; 
if( i_pic_width - i_width == 0 )
{
*pb_hscale = 0;
}
else if( i_pic_width - i_width > 0 )
{
int i_dummy = 0;
*pb_hscale = 1;
i_scale_count = i_pic_width;
for( i_x = i_width; i_x--; )
{
while( (i_scale_count -= i_width) > 0 )
{
*p_offset++ = 0;
*p_offset++ = 0;
}
*p_offset++ = 1;
*p_offset++ = i_dummy;
i_dummy = 1 - i_dummy;
i_scale_count += i_pic_width;
}
}
else 
{
int i_remainder = 0;
int i_jump;
*pb_hscale = 1;
i_scale_count = i_width;
for( i_x = i_pic_width; i_x--; )
{
i_jump = 1;
while( (i_scale_count -= i_pic_width) > 0 )
{
i_jump += 1;
}
*p_offset++ = i_jump;
*p_offset++ = ( i_jump += i_remainder ) >> 1;
i_remainder = i_jump & 1;
i_scale_count += i_width;
}
}
if( i_pic_height - i_height == 0 )
{
*pi_vscale = 0;
}
else if( i_pic_height - i_height > 0 )
{
*pi_vscale = 1;
}
else 
{
*pi_vscale = -1;
}
}
