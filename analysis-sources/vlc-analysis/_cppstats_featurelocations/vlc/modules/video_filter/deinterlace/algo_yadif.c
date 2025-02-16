






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdint.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_cpu.h>
#include <vlc_picture.h>
#include <vlc_filter.h>

#include "deinterlace.h" 
#include "common.h" 

#include "algo_yadif.h"







#include "yadif.h"

int RenderYadifSingle( filter_t *p_filter, picture_t *p_dst, picture_t *p_src )
{
return RenderYadif( p_filter, p_dst, p_src, 0, 0 );
}

int RenderYadif( filter_t *p_filter, picture_t *p_dst, picture_t *p_src,
int i_order, int i_field )
{
VLC_UNUSED(p_src);

filter_sys_t *p_sys = p_filter->p_sys;


assert( i_order >= 0 && i_order <= 2 ); 
assert( i_field == 0 || i_field == 1 );


picture_t *p_prev = p_sys->context.pp_history[0];
picture_t *p_cur = p_sys->context.pp_history[1];
picture_t *p_next = p_sys->context.pp_history[2];
































int yadif_parity;
if( p_cur && p_cur->i_nb_fields > 2 )
yadif_parity = (i_order + 1) % 3; 

else
yadif_parity = (i_order + 1) % 2; 


if( p_prev && p_cur && p_next )
{

void (*filter)(uint8_t *dst, uint8_t *prev, uint8_t *cur, uint8_t *next,
int w, int prefs, int mrefs, int parity, int mode);

#if defined(HAVE_X86ASM)
if( vlc_CPU_SSSE3() )
filter = vlcpriv_yadif_filter_line_ssse3;
else
if( vlc_CPU_SSE2() )
filter = vlcpriv_yadif_filter_line_sse2;
else
#if defined(__i386__)
if( vlc_CPU_MMXEXT() )
filter = vlcpriv_yadif_filter_line_mmxext;
else
#endif
#endif
filter = yadif_filter_line_c;

if( p_sys->chroma->pixel_size == 2 )
filter = yadif_filter_line_c_16bit;

for( int n = 0; n < p_dst->i_planes; n++ )
{
const plane_t *prevp = &p_prev->p[n];
const plane_t *curp = &p_cur->p[n];
const plane_t *nextp = &p_next->p[n];
plane_t *dstp = &p_dst->p[n];

for( int y = 1; y < dstp->i_visible_lines - 1; y++ )
{
if( (y % 2) == i_field || yadif_parity == 2 )
{
memcpy( &dstp->p_pixels[y * dstp->i_pitch],
&curp->p_pixels[y * curp->i_pitch], dstp->i_visible_pitch );
}
else
{
int mode;

mode = (y >= 2 && y < dstp->i_visible_lines - 2) ? 0 : 2;

assert( prevp->i_pitch == curp->i_pitch && curp->i_pitch == nextp->i_pitch );
filter( &dstp->p_pixels[y * dstp->i_pitch],
&prevp->p_pixels[y * prevp->i_pitch],
&curp->p_pixels[y * curp->i_pitch],
&nextp->p_pixels[y * nextp->i_pitch],
dstp->i_visible_pitch,
y < dstp->i_visible_lines - 2 ? curp->i_pitch : -curp->i_pitch,
y - 1 ? -curp->i_pitch : curp->i_pitch,
yadif_parity,
mode );
}


if( y == 1 )
memcpy(&dstp->p_pixels[(y-1) * dstp->i_pitch],
&dstp->p_pixels[ y * dstp->i_pitch],
dstp->i_pitch);
else if( y == dstp->i_visible_lines - 2 )
memcpy(&dstp->p_pixels[(y+1) * dstp->i_pitch],
&dstp->p_pixels[ y * dstp->i_pitch],
dstp->i_pitch);
}
}

p_sys->context.i_frame_offset = 1; 

return VLC_SUCCESS;
}
else if( !p_prev && !p_cur && p_next )
{




RenderX( p_filter, p_dst, p_next );
return VLC_SUCCESS;
}
else
{
p_sys->context.i_frame_offset = 1; 

return VLC_EGENERIC;
}
}
