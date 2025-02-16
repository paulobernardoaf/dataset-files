#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#if defined(CAN_COMPILE_MMXEXT)
#include "mmx.h"
#endif
#include <stdint.h>
#include <vlc_common.h>
#include <vlc_cpu.h>
#include <vlc_picture.h>
#include "deinterlace.h" 
#include "algo_x.h"
static inline int ssd( int a ) { return a*a; }
static inline int XDeint8x8DetectC( uint8_t *src, int i_src )
{
int y, x;
int ff, fr;
int fc;
fc = 0;
for( y = 0; y < 7; y += 2 )
{
ff = fr = 0;
for( x = 0; x < 8; x++ )
{
fr += ssd(src[ x] - src[1*i_src+x]) +
ssd(src[i_src+x] - src[2*i_src+x]);
ff += ssd(src[ x] - src[2*i_src+x]) +
ssd(src[i_src+x] - src[3*i_src+x]);
}
if( ff < 6*fr/8 && fr > 32 )
fc++;
src += 2*i_src;
}
return fc < 1 ? false : true;
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static inline int XDeint8x8DetectMMXEXT( uint8_t *src, int i_src )
{
int y, x;
int32_t ff, fr;
int fc;
fc = 0;
pxor_r2r( mm7, mm7 );
for( y = 0; y < 9; y += 2 )
{
ff = fr = 0;
pxor_r2r( mm5, mm5 );
pxor_r2r( mm6, mm6 );
for( x = 0; x < 8; x+=4 )
{
movd_m2r( src[ x], mm0 );
movd_m2r( src[1*i_src+x], mm1 );
movd_m2r( src[2*i_src+x], mm2 );
movd_m2r( src[3*i_src+x], mm3 );
punpcklbw_r2r( mm7, mm0 );
punpcklbw_r2r( mm7, mm1 );
punpcklbw_r2r( mm7, mm2 );
punpcklbw_r2r( mm7, mm3 );
movq_r2r( mm0, mm4 );
psubw_r2r( mm1, mm0 );
psubw_r2r( mm2, mm4 );
psubw_r2r( mm1, mm2 );
psubw_r2r( mm1, mm3 );
pmaddwd_r2r( mm0, mm0 );
pmaddwd_r2r( mm4, mm4 );
pmaddwd_r2r( mm2, mm2 );
pmaddwd_r2r( mm3, mm3 );
paddd_r2r( mm0, mm2 );
paddd_r2r( mm4, mm3 );
paddd_r2r( mm2, mm5 );
paddd_r2r( mm3, mm6 );
}
movq_r2r( mm5, mm0 );
psrlq_i2r( 32, mm0 );
paddd_r2r( mm0, mm5 );
movd_r2m( mm5, fr );
movq_r2r( mm6, mm0 );
psrlq_i2r( 32, mm0 );
paddd_r2r( mm0, mm6 );
movd_r2m( mm6, ff );
if( ff < 6*fr/8 && fr > 32 )
fc++;
src += 2*i_src;
}
return fc;
}
#endif
static inline void XDeint8x8MergeC( uint8_t *dst, int i_dst,
uint8_t *src1, int i_src1,
uint8_t *src2, int i_src2 )
{
int y, x;
for( y = 0; y < 8; y += 2 )
{
memcpy( dst, src1, 8 );
dst += i_dst;
for( x = 0; x < 8; x++ )
dst[x] = (src1[x] + 6*src2[x] + src1[i_src1+x] + 4 ) >> 3;
dst += i_dst;
src1 += i_src1;
src2 += i_src2;
}
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static inline void XDeint8x8MergeMMXEXT( uint8_t *dst, int i_dst,
uint8_t *src1, int i_src1,
uint8_t *src2, int i_src2 )
{
static const uint64_t m_4 = INT64_C(0x0004000400040004);
int y, x;
pxor_r2r( mm7, mm7 );
for( y = 0; y < 8; y += 2 )
{
for( x = 0; x < 8; x +=4 )
{
movd_m2r( src1[x], mm0 );
movd_r2m( mm0, dst[x] );
movd_m2r( src2[x], mm1 );
movd_m2r( src1[i_src1+x], mm2 );
punpcklbw_r2r( mm7, mm0 );
punpcklbw_r2r( mm7, mm1 );
punpcklbw_r2r( mm7, mm2 );
paddw_r2r( mm1, mm1 );
movq_r2r( mm1, mm3 );
paddw_r2r( mm3, mm3 );
paddw_r2r( mm2, mm0 );
paddw_r2r( mm3, mm1 );
paddw_m2r( m_4, mm1 );
paddw_r2r( mm1, mm0 );
psraw_i2r( 3, mm0 );
packuswb_r2r( mm7, mm0 );
movd_r2m( mm0, dst[i_dst+x] );
}
dst += 2*i_dst;
src1 += i_src1;
src2 += i_src2;
}
}
#endif
static inline void XDeint8x8FieldEC( uint8_t *dst, int i_dst,
uint8_t *src, int i_src )
{
int y, x;
for( y = 0; y < 8; y += 2 )
{
memcpy( dst, src, 8 );
dst += i_dst;
for( x = 0; x < 8; x++ )
dst[x] = (src[x] + src[2*i_src+x] ) >> 1;
dst += 1*i_dst;
src += 2*i_src;
}
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static inline void XDeint8x8FieldEMMXEXT( uint8_t *dst, int i_dst,
uint8_t *src, int i_src )
{
int y;
for( y = 0; y < 8; y += 2 )
{
movq_m2r( src[0], mm0 );
movq_r2m( mm0, dst[0] );
dst += i_dst;
movq_m2r( src[2*i_src], mm1 );
pavgb_r2r( mm1, mm0 );
movq_r2m( mm0, dst[0] );
dst += 1*i_dst;
src += 2*i_src;
}
}
#endif
static inline void XDeint8x8FieldC( uint8_t *dst, int i_dst,
uint8_t *src, int i_src )
{
int y, x;
for( y = 0; y < 8; y += 2 )
{
memcpy( dst, src, 8 );
dst += i_dst;
for( x = 0; x < 8; x++ )
{
uint8_t *src2 = &src[2*i_src];
const int c0 = abs(src[x-4]-src2[x-2]) + abs(src[x-3]-src2[x-1]) +
abs(src[x-2]-src2[x+0]) + abs(src[x-1]-src2[x+1]) +
abs(src[x+0]-src2[x+2]) + abs(src[x+1]-src2[x+3]) +
abs(src[x+2]-src2[x+4]) + abs(src[x+3]-src2[x+5]);
const int c1 = abs(src[x-3]-src2[x-3]) + abs(src[x-2]-src2[x-2]) +
abs(src[x-1]-src2[x-1]) + abs(src[x+0]-src2[x+0]) +
abs(src[x+1]-src2[x+1]) + abs(src[x+2]-src2[x+2]) +
abs(src[x+3]-src2[x+3]) + abs(src[x+4]-src2[x+4]);
const int c2 = abs(src[x-2]-src2[x-4]) + abs(src[x-1]-src2[x-3]) +
abs(src[x+0]-src2[x-2]) + abs(src[x+1]-src2[x-1]) +
abs(src[x+2]-src2[x+0]) + abs(src[x+3]-src2[x+1]) +
abs(src[x+4]-src2[x+2]) + abs(src[x+5]-src2[x+3]);
if( c0 < c1 && c1 <= c2 )
dst[x] = (src[x-1] + src2[x+1]) >> 1;
else if( c2 < c1 && c1 <= c0 )
dst[x] = (src[x+1] + src2[x-1]) >> 1;
else
dst[x] = (src[x+0] + src2[x+0]) >> 1;
}
dst += 1*i_dst;
src += 2*i_src;
}
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static inline void XDeint8x8FieldMMXEXT( uint8_t *dst, int i_dst,
uint8_t *src, int i_src )
{
int y, x;
for( y = 0; y < 8; y += 2 )
{
memcpy( dst, src, 8 );
dst += i_dst;
for( x = 0; x < 8; x++ )
{
uint8_t *src2 = &src[2*i_src];
int32_t c0, c1, c2;
movq_m2r( src[x-2], mm0 );
movq_m2r( src[x-3], mm1 );
movq_m2r( src[x-4], mm2 );
psadbw_m2r( src2[x-4], mm0 );
psadbw_m2r( src2[x-3], mm1 );
psadbw_m2r( src2[x-2], mm2 );
movd_r2m( mm0, c2 );
movd_r2m( mm1, c1 );
movd_r2m( mm2, c0 );
if( c0 < c1 && c1 <= c2 )
dst[x] = (src[x-1] + src2[x+1]) >> 1;
else if( c2 < c1 && c1 <= c0 )
dst[x] = (src[x+1] + src2[x-1]) >> 1;
else
dst[x] = (src[x+0] + src2[x+0]) >> 1;
}
dst += 1*i_dst;
src += 2*i_src;
}
}
#endif
static inline int XDeintNxNDetect( uint8_t *src, int i_src,
int i_height, int i_width )
{
int y, x;
int ff, fr;
int fc;
ff = fr = 0;
fc = 0;
for( y = 0; y < i_height - 2; y += 2 )
{
const uint8_t *s = &src[y*i_src];
for( x = 0; x < i_width; x++ )
{
fr += ssd(s[ x] - s[1*i_src+x]);
ff += ssd(s[ x] - s[2*i_src+x]);
}
if( ff < fr && fr > i_width / 2 )
fc++;
}
return fc < 2 ? false : true;
}
static inline void XDeintNxNFrame( uint8_t *dst, int i_dst,
uint8_t *src, int i_src,
int i_width, int i_height )
{
int y, x;
for( y = 0; y < i_height; y += 2 )
{
memcpy( dst, src, i_width );
dst += i_dst;
if( y < i_height - 2 )
{
for( x = 0; x < i_width; x++ )
dst[x] = (src[x] + 2*src[1*i_src+x] + src[2*i_src+x] + 2 ) >> 2;
}
else
{
for( x = 0; x < i_width; x++ )
dst[x] = (src[x] + src[1*i_src+x] ) >> 1;
}
dst += 1*i_dst;
src += 2*i_src;
}
}
static inline void XDeintNxNField( uint8_t *dst, int i_dst,
uint8_t *src, int i_src,
int i_width, int i_height )
{
int y, x;
for( y = 0; y < i_height; y += 2 )
{
memcpy( dst, src, i_width );
dst += i_dst;
if( y < i_height - 2 )
{
for( x = 0; x < i_width; x++ )
dst[x] = (src[x] + src[2*i_src+x] ) >> 1;
}
else
{
for( x = 0; x < i_width; x++ )
dst[x] = (src[x] + src[i_src+x]) >> 1;
}
dst += 1*i_dst;
src += 2*i_src;
}
}
static inline void XDeintNxN( uint8_t *dst, int i_dst, uint8_t *src, int i_src,
int i_width, int i_height )
{
if( XDeintNxNDetect( src, i_src, i_width, i_height ) )
XDeintNxNField( dst, i_dst, src, i_src, i_width, i_height );
else
XDeintNxNFrame( dst, i_dst, src, i_src, i_width, i_height );
}
static inline void XDeintBand8x8C( uint8_t *dst, int i_dst,
uint8_t *src, int i_src,
const int i_mbx, int i_modx )
{
int x;
for( x = 0; x < i_mbx; x++ )
{
int s;
if( ( s = XDeint8x8DetectC( src, i_src ) ) )
{
if( x == 0 || x == i_mbx - 1 )
XDeint8x8FieldEC( dst, i_dst, src, i_src );
else
XDeint8x8FieldC( dst, i_dst, src, i_src );
}
else
{
XDeint8x8MergeC( dst, i_dst,
&src[0*i_src], 2*i_src,
&src[1*i_src], 2*i_src );
}
dst += 8;
src += 8;
}
if( i_modx )
XDeintNxN( dst, i_dst, src, i_src, i_modx, 8 );
}
#if defined(CAN_COMPILE_MMXEXT)
VLC_MMX
static inline void XDeintBand8x8MMXEXT( uint8_t *dst, int i_dst,
uint8_t *src, int i_src,
const int i_mbx, int i_modx )
{
int x;
for( x = 0; x < i_mbx; x++ )
{
int s;
if( ( s = XDeint8x8DetectMMXEXT( src, i_src ) ) )
{
if( x == 0 || x == i_mbx - 1 )
XDeint8x8FieldEMMXEXT( dst, i_dst, src, i_src );
else
XDeint8x8FieldMMXEXT( dst, i_dst, src, i_src );
}
else
{
XDeint8x8MergeMMXEXT( dst, i_dst,
&src[0*i_src], 2*i_src,
&src[1*i_src], 2*i_src );
}
dst += 8;
src += 8;
}
if( i_modx )
XDeintNxN( dst, i_dst, src, i_src, i_modx, 8 );
}
#endif
int RenderX( filter_t *p_filter, picture_t *p_outpic, picture_t *p_pic )
{
VLC_UNUSED(p_filter);
int i_plane;
#if defined (CAN_COMPILE_MMXEXT)
const bool mmxext = vlc_CPU_MMXEXT();
#endif
for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{
const int i_mby = ( p_outpic->p[i_plane].i_visible_lines + 7 )/8 - 1;
const int i_mbx = p_outpic->p[i_plane].i_visible_pitch/8;
const int i_mody = p_outpic->p[i_plane].i_visible_lines - 8*i_mby;
const int i_modx = p_outpic->p[i_plane].i_visible_pitch - 8*i_mbx;
const int i_dst = p_outpic->p[i_plane].i_pitch;
const int i_src = p_pic->p[i_plane].i_pitch;
int y, x;
for( y = 0; y < i_mby; y++ )
{
uint8_t *dst = &p_outpic->p[i_plane].p_pixels[8*y*i_dst];
uint8_t *src = &p_pic->p[i_plane].p_pixels[8*y*i_src];
#if defined(CAN_COMPILE_MMXEXT)
if( mmxext )
XDeintBand8x8MMXEXT( dst, i_dst, src, i_src, i_mbx, i_modx );
else
#endif
XDeintBand8x8C( dst, i_dst, src, i_src, i_mbx, i_modx );
}
if( i_mody )
{
uint8_t *dst = &p_outpic->p[i_plane].p_pixels[8*y*i_dst];
uint8_t *src = &p_pic->p[i_plane].p_pixels[8*y*i_src];
for( x = 0; x < i_mbx; x++ )
{
XDeintNxN( dst, i_dst, src, i_src, 8, i_mody );
dst += 8;
src += 8;
}
if( i_modx )
XDeintNxN( dst, i_dst, src, i_src, i_modx, i_mody );
}
}
#if defined(CAN_COMPILE_MMXEXT)
if( mmxext )
emms();
#endif
return VLC_SUCCESS;
}
