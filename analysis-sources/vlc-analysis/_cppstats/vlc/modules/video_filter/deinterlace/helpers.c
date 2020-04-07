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
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "deinterlace.h" 
#include "common.h" 
#include "merge.h"
#include "helpers.h"
static void FieldFromPlane( plane_t *p_dst, const plane_t *p_src, int i_field )
{
assert( p_dst != NULL );
assert( p_src != NULL );
assert( i_field == 0 || i_field == 1 );
(*p_dst) = (*p_src);
p_dst->i_lines /= 2;
p_dst->i_visible_lines /= 2;
p_dst->i_pitch *= 2;
if( i_field == 1 )
p_dst->p_pixels += p_src->i_pitch;
}
#define T 10
static int TestForMotionInBlock( uint8_t *p_pix_p, uint8_t *p_pix_c,
int i_pitch_prev, int i_pitch_curr,
int* pi_top, int* pi_bot )
{
int32_t i_motion = 0;
int32_t i_top_motion = 0;
int32_t i_bot_motion = 0;
for( int y = 0; y < 8; ++y )
{
uint8_t *pc = p_pix_c;
uint8_t *pp = p_pix_p;
int score = 0;
for( int x = 0; x < 8; ++x )
{
int_fast16_t C = abs((*pc) - (*pp));
if( C > T )
++score;
++pc;
++pp;
}
i_motion += score;
if( y % 2 == 0 )
i_top_motion += score;
else
i_bot_motion += score;
p_pix_c += i_pitch_curr;
p_pix_p += i_pitch_prev;
}
(*pi_top) = ( i_top_motion >= 8 );
(*pi_bot) = ( i_bot_motion >= 8 );
return (i_motion >= 8);
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static int TestForMotionInBlockMMX( uint8_t *p_pix_p, uint8_t *p_pix_c,
int i_pitch_prev, int i_pitch_curr,
int* pi_top, int* pi_bot )
{
int32_t i_motion = 0;
int32_t i_top_motion = 0;
int32_t i_bot_motion = 0;
static alignas (8) const mmx_t bT = { .ub = { T, T, T, T, T, T, T, T } };
pxor_r2r( mm6, mm6 ); 
movq_m2r( bT, mm5 );
pxor_r2r( mm3, mm3 ); 
pxor_r2r( mm4, mm4 ); 
for( int y = 0; y < 8; y+=2 )
{
movq_m2r( *((uint64_t*)p_pix_c), mm0 );
movq_m2r( *((uint64_t*)p_pix_p), mm1 );
movq_r2r( mm0, mm2 );
psubusb_r2r( mm1, mm2 );
psubusb_r2r( mm0, mm1 );
pcmpgtb_r2r( mm5, mm2 );
pcmpgtb_r2r( mm5, mm1 );
psadbw_r2r( mm6, mm2 );
psadbw_r2r( mm6, mm1 );
paddd_r2r( mm2, mm1 );
paddd_r2r( mm1, mm3 ); 
p_pix_c += i_pitch_curr;
p_pix_p += i_pitch_prev;
movq_m2r( *((uint64_t*)p_pix_c), mm0 );
movq_m2r( *((uint64_t*)p_pix_p), mm1 );
movq_r2r( mm0, mm2 );
psubusb_r2r( mm1, mm2 );
psubusb_r2r( mm0, mm1 );
pcmpgtb_r2r( mm5, mm2 );
pcmpgtb_r2r( mm5, mm1 );
psadbw_r2r( mm6, mm2 );
psadbw_r2r( mm6, mm1 );
paddd_r2r( mm2, mm1 );
paddd_r2r( mm1, mm4 ); 
p_pix_c += i_pitch_curr;
p_pix_p += i_pitch_prev;
}
movq_r2r( mm3, mm7 ); 
paddd_r2r( mm4, mm7 );
movd_r2m( mm3, i_top_motion );
movd_r2m( mm4, i_bot_motion );
movd_r2m( mm7, i_motion );
i_top_motion /= 255;
i_bot_motion /= 255;
i_motion /= 255;
emms();
(*pi_top) = ( i_top_motion >= 8 );
(*pi_bot) = ( i_bot_motion >= 8 );
return (i_motion >= 8);
}
#endif
#undef T
void ComposeFrame( filter_t *p_filter,
picture_t *p_outpic,
picture_t *p_inpic_top, picture_t *p_inpic_bottom,
compose_chroma_t i_output_chroma, bool swapped_uv_conversion )
{
assert( p_outpic != NULL );
assert( p_inpic_top != NULL );
assert( p_inpic_bottom != NULL );
assert( i_output_chroma == CC_ALTLINE ||
i_output_chroma == CC_UPCONVERT ||
i_output_chroma == CC_SOURCE_TOP ||
i_output_chroma == CC_SOURCE_BOTTOM ||
i_output_chroma == CC_MERGE );
filter_sys_t *p_sys = p_filter->p_sys;
const bool b_upconvert_chroma = i_output_chroma == CC_UPCONVERT;
for( int i_plane = 0 ; i_plane < p_inpic_top->i_planes ; i_plane++ )
{
bool b_is_chroma_plane = ( i_plane == U_PLANE || i_plane == V_PLANE );
int i_out_plane;
if( b_is_chroma_plane && b_upconvert_chroma && swapped_uv_conversion )
{
if( i_plane == U_PLANE )
i_out_plane = V_PLANE;
else 
i_out_plane = U_PLANE;
}
else
{
i_out_plane = i_plane;
}
if( !b_is_chroma_plane || i_output_chroma == CC_ALTLINE )
{
plane_t dst_top;
plane_t dst_bottom;
plane_t src_top;
plane_t src_bottom;
FieldFromPlane( &dst_top, &p_outpic->p[i_out_plane], 0 );
FieldFromPlane( &dst_bottom, &p_outpic->p[i_out_plane], 1 );
FieldFromPlane( &src_top, &p_inpic_top->p[i_plane], 0 );
FieldFromPlane( &src_bottom, &p_inpic_bottom->p[i_plane], 1 );
plane_CopyPixels( &dst_top, &src_top );
plane_CopyPixels( &dst_bottom, &src_bottom );
}
else 
{
if( i_output_chroma == CC_UPCONVERT )
{
plane_t dst_top;
plane_t dst_bottom;
FieldFromPlane( &dst_top, &p_outpic->p[i_out_plane], 0 );
FieldFromPlane( &dst_bottom, &p_outpic->p[i_out_plane], 1 );
plane_CopyPixels( &dst_top, &p_inpic_top->p[i_plane] );
plane_CopyPixels( &dst_bottom, &p_inpic_bottom->p[i_plane] );
}
else if( i_output_chroma == CC_SOURCE_TOP )
{
plane_CopyPixels( &p_outpic->p[i_out_plane],
&p_inpic_top->p[i_plane] );
}
else if( i_output_chroma == CC_SOURCE_BOTTOM )
{
plane_CopyPixels( &p_outpic->p[i_out_plane],
&p_inpic_bottom->p[i_plane] );
}
else 
{
uint8_t *p_in_top, *p_in_bottom, *p_out_end, *p_out;
p_in_top = p_inpic_top->p[i_plane].p_pixels;
p_in_bottom = p_inpic_bottom->p[i_plane].p_pixels;
p_out = p_outpic->p[i_out_plane].p_pixels;
p_out_end = p_out + p_outpic->p[i_out_plane].i_pitch
* p_outpic->p[i_out_plane].i_visible_lines;
int w = FFMIN3( p_inpic_top->p[i_plane].i_visible_pitch,
p_inpic_bottom->p[i_plane].i_visible_pitch,
p_outpic->p[i_plane].i_visible_pitch );
for( ; p_out < p_out_end ; )
{
Merge( p_out, p_in_top, p_in_bottom, w );
p_out += p_outpic->p[i_out_plane].i_pitch;
p_in_top += p_inpic_top->p[i_plane].i_pitch;
p_in_bottom += p_inpic_bottom->p[i_plane].i_pitch;
}
EndMerge();
}
}
}
}
int EstimateNumBlocksWithMotion( const picture_t* p_prev,
const picture_t* p_curr,
int *pi_top, int *pi_bot)
{
assert( p_prev != NULL );
assert( p_curr != NULL );
int i_score_top = 0;
int i_score_bot = 0;
if( p_prev->i_planes != p_curr->i_planes )
return -1;
int (*motion_in_block)(uint8_t *, uint8_t *, int , int, int *, int *) =
TestForMotionInBlock;
#if defined(CAN_COMPILE_MMXEXT)
if (vlc_CPU_MMXEXT())
motion_in_block = TestForMotionInBlockMMX;
#endif
int i_score = 0;
for( int i_plane = 0 ; i_plane < p_prev->i_planes ; i_plane++ )
{
if( p_prev->p[i_plane].i_visible_lines !=
p_curr->p[i_plane].i_visible_lines )
return -1;
const int i_pitch_prev = p_prev->p[i_plane].i_pitch;
const int i_pitch_curr = p_curr->p[i_plane].i_pitch;
const int i_mby = p_prev->p[i_plane].i_visible_lines / 8;
const int w = FFMIN( p_prev->p[i_plane].i_visible_pitch,
p_curr->p[i_plane].i_visible_pitch );
const int i_mbx = w / 8;
for( int by = 0; by < i_mby; ++by )
{
uint8_t *p_pix_p = &p_prev->p[i_plane].p_pixels[i_pitch_prev*8*by];
uint8_t *p_pix_c = &p_curr->p[i_plane].p_pixels[i_pitch_curr*8*by];
for( int bx = 0; bx < i_mbx; ++bx )
{
int i_top_temp, i_bot_temp;
i_score += motion_in_block( p_pix_p, p_pix_c,
i_pitch_prev, i_pitch_curr,
&i_top_temp, &i_bot_temp );
i_score_top += i_top_temp;
i_score_bot += i_bot_temp;
p_pix_p += 8;
p_pix_c += 8;
}
}
}
if( pi_top )
(*pi_top) = i_score_top;
if( pi_bot )
(*pi_bot) = i_score_bot;
return i_score;
}
#define T 100
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static int CalculateInterlaceScoreMMX( const picture_t* p_pic_top,
const picture_t* p_pic_bot )
{
assert( p_pic_top->i_planes == p_pic_bot->i_planes );
int32_t i_score_mmx = 0; 
int32_t i_score_c = 0; 
pxor_r2r( mm7, mm7 ); 
for( int i_plane = 0 ; i_plane < p_pic_top->i_planes ; ++i_plane )
{
if( p_pic_top->p[i_plane].i_visible_lines !=
p_pic_bot->p[i_plane].i_visible_lines )
return -1;
const int i_lasty = p_pic_top->p[i_plane].i_visible_lines-1;
const int w = FFMIN( p_pic_top->p[i_plane].i_visible_pitch,
p_pic_bot->p[i_plane].i_visible_pitch );
const int wm8 = w % 8; 
const int w8 = w - wm8; 
const picture_t *cur = p_pic_bot;
const picture_t *ngh = p_pic_top;
int wc = cur->p[i_plane].i_pitch;
int wn = ngh->p[i_plane].i_pitch;
for( int y = 1; y < i_lasty; ++y )
{
uint8_t *p_c = &cur->p[i_plane].p_pixels[y*wc]; 
uint8_t *p_p = &ngh->p[i_plane].p_pixels[(y-1)*wn]; 
uint8_t *p_n = &ngh->p[i_plane].p_pixels[(y+1)*wn]; 
int x = 0;
static alignas (8) const mmx_t b0 = {
.uq = 0x0000000000000000ULL };
static alignas (8) const mmx_t b128 = {
.uq = 0x8080808080808080ULL };
static alignas (8) const mmx_t bT = {
.ub = { T, T, T, T, T, T, T, T } };
for( ; x < w8; x += 8 )
{
movq_m2r( *((int64_t*)p_c), mm0 );
movq_m2r( *((int64_t*)p_p), mm1 );
movq_m2r( *((int64_t*)p_n), mm2 );
psubb_m2r( b128, mm0 );
psubb_m2r( b128, mm1 );
psubb_m2r( b128, mm2 );
psubsb_r2r( mm0, mm1 );
psubsb_r2r( mm0, mm2 );
pxor_r2r( mm3, mm3 );
pxor_r2r( mm4, mm4 );
pxor_r2r( mm5, mm5 );
pxor_r2r( mm6, mm6 );
punpcklbw_r2r( mm1, mm3 );
punpcklbw_r2r( mm2, mm4 );
punpckhbw_r2r( mm1, mm5 );
punpckhbw_r2r( mm2, mm6 );
pmulhw_r2r( mm3, mm4 );
pmulhw_r2r( mm5, mm6 );
packsswb_r2r(mm4, mm6);
pcmpgtb_m2r( bT, mm6 );
psadbw_m2r( b0, mm6 );
paddd_r2r( mm6, mm7 );
p_c += 8;
p_p += 8;
p_n += 8;
}
for( ; x < w; ++x )
{
int_fast32_t C = *p_c;
int_fast32_t P = *p_p;
int_fast32_t N = *p_n;
int_fast32_t comb = (P - C) * (N - C);
if( comb > T )
++i_score_c;
++p_c;
++p_p;
++p_n;
}
const picture_t *tmp = cur;
cur = ngh;
ngh = tmp;
int tmp_pitch = wc;
wc = wn;
wn = tmp_pitch;
}
}
movd_r2m( mm7, i_score_mmx );
emms();
return i_score_mmx/255 + i_score_c;
}
#endif
int CalculateInterlaceScore( const picture_t* p_pic_top,
const picture_t* p_pic_bot )
{
assert( p_pic_top != NULL );
assert( p_pic_bot != NULL );
if( p_pic_top->i_planes != p_pic_bot->i_planes )
return -1;
#if defined(CAN_COMPILE_MMXEXT)
if (vlc_CPU_MMXEXT())
return CalculateInterlaceScoreMMX( p_pic_top, p_pic_bot );
#endif
int32_t i_score = 0;
for( int i_plane = 0 ; i_plane < p_pic_top->i_planes ; ++i_plane )
{
if( p_pic_top->p[i_plane].i_visible_lines !=
p_pic_bot->p[i_plane].i_visible_lines )
return -1;
const int i_lasty = p_pic_top->p[i_plane].i_visible_lines-1;
const int w = FFMIN( p_pic_top->p[i_plane].i_visible_pitch,
p_pic_bot->p[i_plane].i_visible_pitch );
const picture_t *cur = p_pic_bot;
const picture_t *ngh = p_pic_top;
int wc = cur->p[i_plane].i_pitch;
int wn = ngh->p[i_plane].i_pitch;
for( int y = 1; y < i_lasty; ++y )
{
uint8_t *p_c = &cur->p[i_plane].p_pixels[y*wc]; 
uint8_t *p_p = &ngh->p[i_plane].p_pixels[(y-1)*wn]; 
uint8_t *p_n = &ngh->p[i_plane].p_pixels[(y+1)*wn]; 
for( int x = 0; x < w; ++x )
{
int_fast32_t C = *p_c;
int_fast32_t P = *p_p;
int_fast32_t N = *p_n;
int_fast32_t comb = (P - C) * (N - C);
if( comb > T )
++i_score;
++p_c;
++p_p;
++p_n;
}
const picture_t *tmp = cur;
cur = ngh;
ngh = tmp;
int tmp_pitch = wc;
wc = wn;
wn = tmp_pitch;
}
}
return i_score;
}
#undef T
