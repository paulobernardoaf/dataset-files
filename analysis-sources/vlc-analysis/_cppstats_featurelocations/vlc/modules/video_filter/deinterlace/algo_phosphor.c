





















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(CAN_COMPILE_MMXEXT)
#include "mmx.h"
#include <stdalign.h>
#endif

#include <stdint.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_cpu.h>
#include <vlc_picture.h>
#include <vlc_filter.h>

#include "deinterlace.h" 
#include "helpers.h" 

#include "algo_phosphor.h"



























static void DarkenField( picture_t *p_dst,
const int i_field, const int i_strength,
bool process_chroma )
{
assert( p_dst != NULL );
assert( i_field == 0 || i_field == 1 );
assert( i_strength >= 1 && i_strength <= 3 );



const uint8_t remove_high_u8 = 0xFF >> i_strength;
const uint64_t remove_high_u64 = remove_high_u8 *
INT64_C(0x0101010101010101);








int i_plane = Y_PLANE;
uint8_t *p_out, *p_out_end;
int w = p_dst->p[i_plane].i_visible_pitch;
p_out = p_dst->p[i_plane].p_pixels;
p_out_end = p_out + p_dst->p[i_plane].i_pitch
* p_dst->p[i_plane].i_visible_lines;


if( i_field == 1 )
p_out += p_dst->p[i_plane].i_pitch;

int wm8 = w % 8; 
int w8 = w - wm8; 
for( ; p_out < p_out_end ; p_out += 2*p_dst->p[i_plane].i_pitch )
{
uint64_t *po = (uint64_t *)p_out;
int x = 0;

for( ; x < w8; x += 8, ++po )
(*po) = ( ((*po) >> i_strength) & remove_high_u64 );


uint8_t *po_temp = (uint8_t *)po;
for( ; x < w; ++x, ++po_temp )
(*po_temp) = ( ((*po_temp) >> i_strength) & remove_high_u8 );
}







if( process_chroma )
{
for( i_plane++ ;
i_plane < p_dst->i_planes;
i_plane++ )
{
w = p_dst->p[i_plane].i_visible_pitch;
p_out = p_dst->p[i_plane].p_pixels;
p_out_end = p_out + p_dst->p[i_plane].i_pitch
* p_dst->p[i_plane].i_visible_lines;


if( i_field == 1 )
p_out += p_dst->p[i_plane].i_pitch;

for( ; p_out < p_out_end ; p_out += 2*p_dst->p[i_plane].i_pitch )
{

uint8_t *po = p_out;
for( int x = 0; x < w; ++x, ++po )
(*po) = 128 + ( ((*po) - 128) / (1 << i_strength) );
} 
} 
} 
}

#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static void DarkenFieldMMX( picture_t *p_dst,
const int i_field, const int i_strength,
bool process_chroma )
{
assert( p_dst != NULL );
assert( i_field == 0 || i_field == 1 );
assert( i_strength >= 1 && i_strength <= 3 );

uint64_t i_strength_u64 = i_strength; 
const uint8_t remove_high_u8 = 0xFF >> i_strength;
const uint64_t remove_high_u64 = remove_high_u8 *
INT64_C(0x0101010101010101);

int i_plane = Y_PLANE;
uint8_t *p_out, *p_out_end;
int w = p_dst->p[i_plane].i_visible_pitch;
p_out = p_dst->p[i_plane].p_pixels;
p_out_end = p_out + p_dst->p[i_plane].i_pitch
* p_dst->p[i_plane].i_visible_lines;


if( i_field == 1 )
p_out += p_dst->p[i_plane].i_pitch;

int wm8 = w % 8; 
int w8 = w - wm8; 
for( ; p_out < p_out_end ; p_out += 2*p_dst->p[i_plane].i_pitch )
{
uint64_t *po = (uint64_t *)p_out;
int x = 0;

movq_m2r( i_strength_u64, mm1 );
movq_m2r( remove_high_u64, mm2 );
for( ; x < w8; x += 8 )
{
movq_m2r( (*po), mm0 );

psrlq_r2r( mm1, mm0 );
pand_r2r( mm2, mm0 );

movq_r2m( mm0, (*po++) );
}


uint8_t *po_temp = (uint8_t *)po;
for( ; x < w; ++x, ++po_temp )
(*po_temp) = ( ((*po_temp) >> i_strength) & remove_high_u8 );
}







if( process_chroma )
{
for( i_plane++ ;
i_plane < p_dst->i_planes;
i_plane++ )
{
w = p_dst->p[i_plane].i_visible_pitch;
wm8 = w % 8; 
w8 = w - wm8; 

p_out = p_dst->p[i_plane].p_pixels;
p_out_end = p_out + p_dst->p[i_plane].i_pitch
* p_dst->p[i_plane].i_visible_lines;


if( i_field == 1 )
p_out += p_dst->p[i_plane].i_pitch;

for( ; p_out < p_out_end ; p_out += 2*p_dst->p[i_plane].i_pitch )
{
int x = 0;


static alignas (8) const mmx_t b128 = {
.uq = 0x8080808080808080ULL
};

movq_m2r( b128, mm5 );
movq_m2r( i_strength_u64, mm6 );
movq_m2r( remove_high_u64, mm7 );

uint64_t *po8 = (uint64_t *)p_out;
for( ; x < w8; x += 8 )
{
movq_m2r( (*po8), mm0 );

movq_r2r( mm5, mm2 ); 
movq_r2r( mm0, mm1 ); 
psubusb_r2r( mm2, mm1 ); 
psubusb_r2r( mm0, mm2 ); 


psrlq_r2r( mm6, mm1 );
psrlq_r2r( mm6, mm2 );
pand_r2r( mm7, mm1 );
pand_r2r( mm7, mm2 );


psubb_r2r( mm2, mm1 );
paddb_r2r( mm5, mm1 );

movq_r2m( mm1, (*po8++) );
}


uint8_t *po = p_out;
for( ; x < w; ++x, ++po )
(*po) = 128 + ( ((*po) - 128) / (1 << i_strength) );
} 
} 
} 

emms();
}
#endif






int RenderPhosphor( filter_t *p_filter,
picture_t *p_dst, picture_t *p_pic,
int i_order, int i_field )
{
VLC_UNUSED(p_pic);
assert( p_filter != NULL );
assert( p_dst != NULL );
assert( i_order >= 0 && i_order <= 2 ); 
assert( i_field == 0 || i_field == 1 );

filter_sys_t *p_sys = p_filter->p_sys;


picture_t *p_in = p_sys->context.pp_history[HISTORY_SIZE-1];
picture_t *p_old = p_sys->context.pp_history[HISTORY_SIZE-2];


if( !p_old )
p_old = p_in;


if( !p_in )
return VLC_EGENERIC;

assert( p_old != NULL );
assert( p_in != NULL );


picture_t *p_in_top = p_in;
picture_t *p_in_bottom = p_in;


if( i_order == 0 )
{
if( i_field == 0 ) 
p_in_bottom = p_old;
else 
p_in_top = p_old;
}

compose_chroma_t cc = CC_ALTLINE;
if( 2 * p_sys->chroma->p[1].h.num == p_sys->chroma->p[1].h.den &&
2 * p_sys->chroma->p[2].h.num == p_sys->chroma->p[2].h.den )
{

switch( p_sys->phosphor.i_chroma_for_420 )
{
case PC_BLEND:
cc = CC_MERGE;
break;
case PC_LATEST:
if( i_field == 0 )
cc = CC_SOURCE_TOP;
else 
cc = CC_SOURCE_BOTTOM;
break;
case PC_ALTLINE:
cc = CC_ALTLINE;
break;
case PC_UPCONVERT:
cc = CC_UPCONVERT;
break;
default:

vlc_assert_unreachable();
break;
}
}
ComposeFrame( p_filter, p_dst, p_in_top, p_in_bottom, cc, p_filter->fmt_in.video.i_chroma == VLC_CODEC_YV12 );










if( p_sys->phosphor.i_dimmer_strength > 0 )
{
#if defined(CAN_COMPILE_MMXEXT)
if( vlc_CPU_MMXEXT() )
DarkenFieldMMX( p_dst, !i_field, p_sys->phosphor.i_dimmer_strength,
p_sys->chroma->p[1].h.num == p_sys->chroma->p[1].h.den &&
p_sys->chroma->p[2].h.num == p_sys->chroma->p[2].h.den );
else
#endif
DarkenField( p_dst, !i_field, p_sys->phosphor.i_dimmer_strength,
p_sys->chroma->p[1].h.num == p_sys->chroma->p[1].h.den &&
p_sys->chroma->p[2].h.num == p_sys->chroma->p[2].h.den );
}
return VLC_SUCCESS;
}
