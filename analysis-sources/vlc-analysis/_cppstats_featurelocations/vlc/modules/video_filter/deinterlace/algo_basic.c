























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdint.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_picture.h>
#include <vlc_filter.h>

#include "merge.h"
#include "deinterlace.h" 

#include "algo_basic.h"





int RenderDiscard( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic )
{
VLC_UNUSED(p_filter);
int i_plane;


for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
uint8_t *p_in, *p_out_end, *p_out;

p_in = p_pic->p[i_plane].p_pixels;

p_out = p_outpic->p[i_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_plane].i_pitch
* p_outpic->p[i_plane].i_visible_lines;

for( ; p_out < p_out_end ; )
{
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

p_out += p_outpic->p[i_plane].i_pitch;
p_in += 2 * p_pic->p[i_plane].i_pitch;
}
}
return VLC_SUCCESS;
}





int RenderBob( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic,
int order, int i_field )
{
VLC_UNUSED(p_filter);
VLC_UNUSED(order);
int i_plane;


for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
uint8_t *p_in, *p_out_end, *p_out;

p_in = p_pic->p[i_plane].p_pixels;
p_out = p_outpic->p[i_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_plane].i_pitch
* p_outpic->p[i_plane].i_visible_lines;


if( i_field == 1 )
{
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
p_in += p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
}

p_out_end -= 2 * p_outpic->p[i_plane].i_pitch;

for( ; p_out < p_out_end ; )
{
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

p_out += p_outpic->p[i_plane].i_pitch;

memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

p_in += 2 * p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
}

memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );


if( i_field == 0 )
{
p_in += p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
}
}
return VLC_SUCCESS;
}





int RenderLinear( filter_t *p_filter,
picture_t *p_outpic, picture_t *p_pic, int order, int i_field )
{
VLC_UNUSED(order);
int i_plane;

filter_sys_t *p_sys = p_filter->p_sys;


for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
uint8_t *p_in, *p_out_end, *p_out;

p_in = p_pic->p[i_plane].p_pixels;
p_out = p_outpic->p[i_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_plane].i_pitch
* p_outpic->p[i_plane].i_visible_lines;


if( i_field == 1 )
{
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
p_in += p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
}

p_out_end -= 2 * p_outpic->p[i_plane].i_pitch;

for( ; p_out < p_out_end ; )
{
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

p_out += p_outpic->p[i_plane].i_pitch;

Merge( p_out, p_in, p_in + 2 * p_pic->p[i_plane].i_pitch,
p_pic->p[i_plane].i_pitch );

p_in += 2 * p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
}

memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );


if( i_field == 0 )
{
p_in += p_pic->p[i_plane].i_pitch;
p_out += p_outpic->p[i_plane].i_pitch;
memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
}
}
EndMerge();
return VLC_SUCCESS;
}





int RenderMean( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic )
{
int i_plane;

filter_sys_t *p_sys = p_filter->p_sys;


for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
uint8_t *p_in, *p_out_end, *p_out;

p_in = p_pic->p[i_plane].p_pixels;

p_out = p_outpic->p[i_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_plane].i_pitch
* p_outpic->p[i_plane].i_visible_lines;


for( ; p_out < p_out_end ; )
{
Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
p_pic->p[i_plane].i_pitch );

p_out += p_outpic->p[i_plane].i_pitch;
p_in += 2 * p_pic->p[i_plane].i_pitch;
}
}
EndMerge();
return VLC_SUCCESS;
}





int RenderBlend( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic )
{
int i_plane;

filter_sys_t *p_sys = p_filter->p_sys;


for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
uint8_t *p_in, *p_out_end, *p_out;

p_in = p_pic->p[i_plane].p_pixels;

p_out = p_outpic->p[i_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_plane].i_pitch
* p_outpic->p[i_plane].i_visible_lines;


memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
p_out += p_outpic->p[i_plane].i_pitch;


for( ; p_out < p_out_end ; )
{
Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
p_pic->p[i_plane].i_pitch );

p_out += p_outpic->p[i_plane].i_pitch;
p_in += p_pic->p[i_plane].i_pitch;
}
}
EndMerge();
return VLC_SUCCESS;
}
