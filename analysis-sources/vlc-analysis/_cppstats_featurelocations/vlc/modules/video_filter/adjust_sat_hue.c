























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
#include "adjust_sat_hue.h"

#define I_RANGE( i_bpp ) (1 << i_bpp)
#define I_MAX( i_bpp ) (I_RANGE( i_bpp ) - 1)
#define I_MID( i_bpp ) (I_RANGE( i_bpp ) >> 1)

#define PLANAR_WRITE_UV_CLIP( i_bpp ) i_u = *p_in++ ; i_v = *p_in_v++ ; *p_out++ = VLC_CLIP( ((( (((i_u * i_cos + i_v * i_sin - i_x) + I_MID( i_bpp )) >> i_bpp) * i_sat) + I_MID( i_bpp )) >> i_bpp) + I_MID( i_bpp ), 0, I_MAX( i_bpp ) ); *p_out_v++ = VLC_CLIP( ((( (((i_v * i_cos - i_u * i_sin - i_y) + I_MID( i_bpp )) >> i_bpp) * i_sat) + I_MID( i_bpp )) >> i_bpp) + I_MID( i_bpp ), 0, I_MAX( i_bpp ) )






#define PLANAR_WRITE_UV( i_bpp ) i_u = *p_in++ ; i_v = *p_in_v++ ; *p_out++ = ((( (((i_u * i_cos + i_v * i_sin - i_x) + I_MID( i_bpp )) >> i_bpp) * i_sat) + I_MID( i_bpp )) >> i_bpp) + I_MID( i_bpp ); *p_out_v++ = ((( (((i_v * i_cos - i_u * i_sin - i_y) + I_MID( i_bpp )) >> i_bpp) * i_sat) + I_MID( i_bpp )) >> i_bpp) + I_MID( i_bpp )






#define PACKED_WRITE_UV_CLIP() i_u = *p_in; p_in += 4; i_v = *p_in_v; p_in_v += 4; *p_out = clip_uint8_vlc( ((( (((i_u * i_cos + i_v * i_sin - i_x) + I_MID( 8 )) >> 8) * i_sat) + I_MID( 8 )) >> 8) + 128); p_out += 4; *p_out_v = clip_uint8_vlc( ((( (((i_v * i_cos - i_u * i_sin - i_y) + I_MID( 8 )) >> 8) * i_sat) + I_MID( 8 )) >> 8) + 128); p_out_v += 4








#define PACKED_WRITE_UV() i_u = *p_in; p_in += 4; i_v = *p_in_v; p_in_v += 4; *p_out = ((( (((i_u * i_cos + i_v * i_sin - i_x) + I_MID( 8 )) >> 8) * i_sat) + I_MID( 8 )) >> 8) + 128; p_out += 4; *p_out_v = ((( (((i_v * i_cos - i_u * i_sin - i_y) + I_MID( 8 )) >> 8) * i_sat) + I_MID( 8 )) >> 8) + 128; p_out_v += 4








#define ADJUST_8_TIMES(x) x; x; x; x; x; x; x; x





int planar_sat_hue_clip_C( picture_t * p_pic, picture_t * p_outpic, int i_sin, int i_cos,
int i_sat, int i_x, int i_y )
{
uint8_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint8_t *p_out, *p_out_v;

p_in = p_pic->p[U_PLANE].p_pixels;
p_in_v = p_pic->p[V_PLANE].p_pixels;
p_in_end = p_in + p_pic->p[U_PLANE].i_visible_lines
* p_pic->p[U_PLANE].i_pitch - 8;

p_out = p_outpic->p[U_PLANE].p_pixels;
p_out_v = p_outpic->p[V_PLANE].p_pixels;

uint8_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + p_pic->p[U_PLANE].i_visible_pitch - 8;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PLANAR_WRITE_UV_CLIP( 8 ) );
}

p_line_end += 8;

for( ; p_in < p_line_end ; )
{
PLANAR_WRITE_UV_CLIP( 8 );
}

p_in += p_pic->p[U_PLANE].i_pitch
- p_pic->p[U_PLANE].i_visible_pitch;
p_in_v += p_pic->p[V_PLANE].i_pitch
- p_pic->p[V_PLANE].i_visible_pitch;
p_out += p_outpic->p[U_PLANE].i_pitch
- p_outpic->p[U_PLANE].i_visible_pitch;
p_out_v += p_outpic->p[V_PLANE].i_pitch
- p_outpic->p[V_PLANE].i_visible_pitch;
}

return VLC_SUCCESS;
}

int planar_sat_hue_C( picture_t * p_pic, picture_t * p_outpic, int i_sin, int i_cos,
int i_sat, int i_x, int i_y )
{
uint8_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint8_t *p_out, *p_out_v;

p_in = p_pic->p[U_PLANE].p_pixels;
p_in_v = p_pic->p[V_PLANE].p_pixels;
p_in_end = p_in + p_pic->p[U_PLANE].i_visible_lines
* p_pic->p[U_PLANE].i_pitch - 8;

p_out = p_outpic->p[U_PLANE].p_pixels;
p_out_v = p_outpic->p[V_PLANE].p_pixels;

uint8_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + p_pic->p[U_PLANE].i_visible_pitch - 8;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PLANAR_WRITE_UV( 8 ) );
}

p_line_end += 8;

for( ; p_in < p_line_end ; )
{
PLANAR_WRITE_UV( 8 );
}

p_in += p_pic->p[U_PLANE].i_pitch
- p_pic->p[U_PLANE].i_visible_pitch;
p_in_v += p_pic->p[V_PLANE].i_pitch
- p_pic->p[V_PLANE].i_visible_pitch;
p_out += p_outpic->p[U_PLANE].i_pitch
- p_outpic->p[U_PLANE].i_visible_pitch;
p_out_v += p_outpic->p[V_PLANE].i_pitch
- p_outpic->p[V_PLANE].i_visible_pitch;
}

return VLC_SUCCESS;
}

int planar_sat_hue_clip_C_16( picture_t * p_pic, picture_t * p_outpic, int i_sin, int i_cos,
int i_sat, int i_x, int i_y )
{
uint16_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint16_t *p_out, *p_out_v;

int i_bpp;
switch( p_pic->format.i_chroma )
{
CASE_PLANAR_YUV10
i_bpp = 10;
break;
CASE_PLANAR_YUV9
i_bpp = 9;
break;
default:
vlc_assert_unreachable();
}

p_in = (uint16_t *) p_pic->p[U_PLANE].p_pixels;
p_in_v = (uint16_t *) p_pic->p[V_PLANE].p_pixels;
p_in_end = p_in + p_pic->p[U_PLANE].i_visible_lines
* (p_pic->p[U_PLANE].i_pitch >> 1) - 8;

p_out = (uint16_t *) p_outpic->p[U_PLANE].p_pixels;
p_out_v = (uint16_t *) p_outpic->p[V_PLANE].p_pixels;

uint16_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + (p_pic->p[U_PLANE].i_visible_pitch >> 1) - 8;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PLANAR_WRITE_UV_CLIP( i_bpp ) );
}

p_line_end += 8;

for( ; p_in < p_line_end ; )
{
PLANAR_WRITE_UV_CLIP( i_bpp );
}

p_in += (p_pic->p[U_PLANE].i_pitch >> 1)
- (p_pic->p[U_PLANE].i_visible_pitch >> 1);
p_in_v += (p_pic->p[V_PLANE].i_pitch >> 1)
- (p_pic->p[V_PLANE].i_visible_pitch >> 1);
p_out += (p_outpic->p[U_PLANE].i_pitch >> 1)
- (p_outpic->p[U_PLANE].i_visible_pitch >> 1);
p_out_v += (p_outpic->p[V_PLANE].i_pitch >> 1)
- (p_outpic->p[V_PLANE].i_visible_pitch >> 1);
}

return VLC_SUCCESS;
}

int planar_sat_hue_C_16( picture_t * p_pic, picture_t * p_outpic, int i_sin, int i_cos,
int i_sat, int i_x, int i_y )
{
uint16_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint16_t *p_out, *p_out_v;

int i_bpp;
switch( p_pic->format.i_chroma )
{
CASE_PLANAR_YUV10
i_bpp = 10;
break;
CASE_PLANAR_YUV9
i_bpp = 9;
break;
default:
vlc_assert_unreachable();
}

p_in = (uint16_t *) p_pic->p[U_PLANE].p_pixels;
p_in_v = (uint16_t *) p_pic->p[V_PLANE].p_pixels;
p_in_end = (uint16_t *) p_in + p_pic->p[U_PLANE].i_visible_lines
* (p_pic->p[U_PLANE].i_pitch >> 1) - 8;

p_out = (uint16_t *) p_outpic->p[U_PLANE].p_pixels;
p_out_v = (uint16_t *) p_outpic->p[V_PLANE].p_pixels;

uint16_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + (p_pic->p[U_PLANE].i_visible_pitch >> 1) - 8;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PLANAR_WRITE_UV( i_bpp ) );
}

p_line_end += 8;

for( ; p_in < p_line_end ; )
{
PLANAR_WRITE_UV( i_bpp );
}

p_in += (p_pic->p[U_PLANE].i_pitch >> 1)
- (p_pic->p[U_PLANE].i_visible_pitch >> 1);
p_in_v += (p_pic->p[V_PLANE].i_pitch >> 1)
- (p_pic->p[V_PLANE].i_visible_pitch >> 1);
p_out += (p_outpic->p[U_PLANE].i_pitch >> 1)
- (p_outpic->p[U_PLANE].i_visible_pitch >> 1);
p_out_v += (p_outpic->p[V_PLANE].i_pitch >> 1)
- (p_outpic->p[V_PLANE].i_visible_pitch >> 1);
}

return VLC_SUCCESS;
}

int packed_sat_hue_clip_C( picture_t * p_pic, picture_t * p_outpic, int i_sin, int i_cos,
int i_sat, int i_x, int i_y )
{
uint8_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint8_t *p_out, *p_out_v;

int i_y_offset, i_u_offset, i_v_offset;
int i_visible_lines, i_pitch, i_visible_pitch;


if ( GetPackedYuvOffsets( p_pic->format.i_chroma, &i_y_offset,
&i_u_offset, &i_v_offset ) != VLC_SUCCESS )
return VLC_EGENERIC;

i_visible_lines = p_pic->p->i_visible_lines;
i_pitch = p_pic->p->i_pitch;
i_visible_pitch = p_pic->p->i_visible_pitch;

p_in = p_pic->p->p_pixels + i_u_offset;
p_in_v = p_pic->p->p_pixels + i_v_offset;
p_in_end = p_in + i_visible_lines * i_pitch - 8 * 4;

p_out = p_outpic->p->p_pixels + i_u_offset;
p_out_v = p_outpic->p->p_pixels + i_v_offset;

uint8_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + i_visible_pitch - 8 * 4;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PACKED_WRITE_UV_CLIP() );
}

p_line_end += 8 * 4;

for( ; p_in < p_line_end ; )
{
PACKED_WRITE_UV_CLIP();
}

p_in += i_pitch - i_visible_pitch;
p_in_v += i_pitch - i_visible_pitch;
p_out += i_pitch - i_visible_pitch;
p_out_v += i_pitch - i_visible_pitch;
}

return VLC_SUCCESS;
}

int packed_sat_hue_C( picture_t * p_pic, picture_t * p_outpic, int i_sin,
int i_cos, int i_sat, int i_x, int i_y )
{
uint8_t *p_in, *p_in_v, *p_in_end, *p_line_end;
uint8_t *p_out, *p_out_v;

int i_y_offset, i_u_offset, i_v_offset;
int i_visible_lines, i_pitch, i_visible_pitch;


if ( GetPackedYuvOffsets( p_pic->format.i_chroma, &i_y_offset,
&i_u_offset, &i_v_offset ) != VLC_SUCCESS )
return VLC_EGENERIC;

i_visible_lines = p_pic->p->i_visible_lines;
i_pitch = p_pic->p->i_pitch;
i_visible_pitch = p_pic->p->i_visible_pitch;

p_in = p_pic->p->p_pixels + i_u_offset;
p_in_v = p_pic->p->p_pixels + i_v_offset;
p_in_end = p_in + i_visible_lines * i_pitch - 8 * 4;

p_out = p_outpic->p->p_pixels + i_u_offset;
p_out_v = p_outpic->p->p_pixels + i_v_offset;

uint8_t i_u, i_v;

for( ; p_in < p_in_end ; )
{
p_line_end = p_in + i_visible_pitch - 8 * 4;

for( ; p_in < p_line_end ; )
{

ADJUST_8_TIMES( PACKED_WRITE_UV() );
}

p_line_end += 8 * 4;

for( ; p_in < p_line_end ; )
{
PACKED_WRITE_UV();
}

p_in += i_pitch - i_visible_pitch;
p_in_v += i_pitch - i_visible_pitch;
p_out += i_pitch - i_visible_pitch;
p_out_v += i_pitch - i_visible_pitch;
}

return VLC_SUCCESS;
}
