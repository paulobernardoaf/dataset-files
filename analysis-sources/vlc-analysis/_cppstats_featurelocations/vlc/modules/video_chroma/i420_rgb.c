


























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_cpu.h>

#include "i420_rgb.h"
#if defined(PLAIN)
#include "i420_rgb_c.h"
static picture_t *I420_RGB8_Filter( filter_t *, picture_t * );
static picture_t *I420_RGB16_Filter( filter_t *, picture_t * );
static picture_t *I420_RGB32_Filter( filter_t *, picture_t * );

static void SetYUV( filter_t *, const video_format_t * );
static void Set8bppPalette( filter_t *, uint8_t * );
#else
static picture_t *I420_R5G5B5_Filter( filter_t *, picture_t * );
static picture_t *I420_R5G6B5_Filter( filter_t *, picture_t * );
static picture_t *I420_A8R8G8B8_Filter( filter_t *, picture_t * );
static picture_t *I420_R8G8B8A8_Filter( filter_t *, picture_t * );
static picture_t *I420_B8G8R8A8_Filter( filter_t *, picture_t * );
static picture_t *I420_A8B8G8R8_Filter( filter_t *, picture_t * );
#endif




#define RGB2PIXEL( p_filter, i_r, i_g, i_b ) ((((i_r) >> i_rrshift) << i_lrshift) | (((i_g) >> i_rgshift) << i_lgshift) | (((i_b) >> i_rbshift) << i_lbshift))







static int Activate ( vlc_object_t * );
static void Deactivate ( vlc_object_t * );

vlc_module_begin ()
#if defined (SSE2)
set_description( N_( "SSE2 I420,IYUV,YV12 to "
"RV15,RV16,RV24,RV32 conversions") )
set_capability( "video converter", 120 )
#define vlc_CPU_capable() vlc_CPU_SSE2()
#elif defined (MMX)
set_description( N_( "MMX I420,IYUV,YV12 to "
"RV15,RV16,RV24,RV32 conversions") )
set_capability( "video converter", 100 )
#define vlc_CPU_capable() vlc_CPU_MMX()
#else
set_description( N_("I420,IYUV,YV12 to "
"RGB8,RV15,RV16,RV24,RV32 conversions") )
set_capability( "video converter", 80 )
#define vlc_CPU_capable() (true)
#endif
set_callbacks( Activate, Deactivate )
vlc_module_end ()






static int Activate( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
#if defined(PLAIN)
size_t i_tables_size;
#endif

if( !vlc_CPU_capable() )
return VLC_EGENERIC;
if( p_filter->fmt_out.video.i_width & 1
|| p_filter->fmt_out.video.i_height & 1 )
{
return VLC_EGENERIC;
}

if( p_filter->fmt_in.video.orientation != p_filter->fmt_out.video.orientation )
{
return VLC_EGENERIC;
}

switch( p_filter->fmt_in.video.i_chroma )
{
case VLC_CODEC_YV12:
case VLC_CODEC_I420:
switch( p_filter->fmt_out.video.i_chroma )
{
#if !defined(PLAIN)
case VLC_CODEC_RGB15:
case VLC_CODEC_RGB16:

if( ( p_filter->fmt_out.video.i_rmask == 0x7c00
&& p_filter->fmt_out.video.i_gmask == 0x03e0
&& p_filter->fmt_out.video.i_bmask == 0x001f ) )
{

msg_Dbg(p_this, "RGB pixel format is R5G5B5");
p_filter->pf_video_filter = I420_R5G5B5_Filter;
}
else if( ( p_filter->fmt_out.video.i_rmask == 0xf800
&& p_filter->fmt_out.video.i_gmask == 0x07e0
&& p_filter->fmt_out.video.i_bmask == 0x001f ) )
{

msg_Dbg(p_this, "RGB pixel format is R5G6B5");
p_filter->pf_video_filter = I420_R5G6B5_Filter;
}
else
return VLC_EGENERIC;
break;
case VLC_CODEC_RGB32:

if( p_filter->fmt_out.video.i_rmask == 0x00ff0000
&& p_filter->fmt_out.video.i_gmask == 0x0000ff00
&& p_filter->fmt_out.video.i_bmask == 0x000000ff )
{

msg_Dbg(p_this, "RGB pixel format is A8R8G8B8");
p_filter->pf_video_filter = I420_A8R8G8B8_Filter;
}
else if( p_filter->fmt_out.video.i_rmask == 0xff000000
&& p_filter->fmt_out.video.i_gmask == 0x00ff0000
&& p_filter->fmt_out.video.i_bmask == 0x0000ff00 )
{

msg_Dbg(p_this, "RGB pixel format is R8G8B8A8");
p_filter->pf_video_filter = I420_R8G8B8A8_Filter;
}
else if( p_filter->fmt_out.video.i_rmask == 0x0000ff00
&& p_filter->fmt_out.video.i_gmask == 0x00ff0000
&& p_filter->fmt_out.video.i_bmask == 0xff000000 )
{

msg_Dbg(p_this, "RGB pixel format is B8G8R8A8");
p_filter->pf_video_filter = I420_B8G8R8A8_Filter;
}
else if( p_filter->fmt_out.video.i_rmask == 0x000000ff
&& p_filter->fmt_out.video.i_gmask == 0x0000ff00
&& p_filter->fmt_out.video.i_bmask == 0x00ff0000 )
{

msg_Dbg(p_this, "RGB pixel format is A8B8G8R8");
p_filter->pf_video_filter = I420_A8B8G8R8_Filter;
}
else
return VLC_EGENERIC;
break;
#else
case VLC_CODEC_RGB8:
p_filter->pf_video_filter = I420_RGB8_Filter;
break;
case VLC_CODEC_RGB15:
case VLC_CODEC_RGB16:
p_filter->pf_video_filter = I420_RGB16_Filter;
break;
case VLC_CODEC_RGB32:
p_filter->pf_video_filter = I420_RGB32_Filter;
break;
#endif
default:
return VLC_EGENERIC;
}
break;

default:
return VLC_EGENERIC;
}

filter_sys_t *p_sys = malloc( sizeof( filter_sys_t ) );
if( p_sys == NULL )
return VLC_EGENERIC;
p_filter->p_sys = p_sys;

p_sys->i_buffer_size = 0;
p_sys->p_buffer = NULL;
switch( p_filter->fmt_out.video.i_chroma )
{
#if defined(PLAIN)
case VLC_CODEC_RGB8:
p_sys->i_bytespp = 1;
break;
#endif
case VLC_CODEC_RGB15:
case VLC_CODEC_RGB16:
p_sys->i_bytespp = 2;
break;
case VLC_CODEC_RGB24:
case VLC_CODEC_RGB32:
p_sys->i_bytespp = 4;
break;
default:
free( p_sys );
return VLC_EGENERIC;
}

p_sys->p_offset = malloc( p_filter->fmt_out.video.i_width
* ( ( p_filter->fmt_out.video.i_chroma
== VLC_CODEC_RGB8 ) ? 2 : 1 )
* sizeof( int ) );
if( p_sys->p_offset == NULL )
{
free( p_sys );
return VLC_EGENERIC;
}

#if defined(PLAIN)
switch( p_filter->fmt_out.video.i_chroma )
{
case VLC_CODEC_RGB8:
i_tables_size = sizeof( uint8_t ) * PALETTE_TABLE_SIZE;
break;
case VLC_CODEC_RGB15:
case VLC_CODEC_RGB16:
i_tables_size = sizeof( uint16_t ) * RGB_TABLE_SIZE;
break;
default: 
i_tables_size = sizeof( uint32_t ) * RGB_TABLE_SIZE;
break;
}

p_sys->p_base = malloc( i_tables_size );
if( p_sys->p_base == NULL )
{
free( p_sys->p_offset );
free( p_sys );
return -1;
}

video_format_t vfmt;
video_format_Init( &vfmt, p_filter->fmt_out.video.i_chroma );
video_format_Copy( &vfmt, &p_filter->fmt_out.video );
if( !vfmt.i_bmask || !vfmt.i_gmask || !vfmt.i_bmask )
msg_Warn( p_filter, "source did not set proper target RGB masks, using default" );
video_format_FixRgb( &vfmt );
SetYUV( p_filter, &vfmt );
video_format_Clean( &vfmt );
#endif

return 0;
}






static void Deactivate( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;

#if defined(PLAIN)
free( p_sys->p_base );
#endif
free( p_sys->p_offset );
free( p_sys->p_buffer );
free( p_sys );
}

#if !defined(PLAIN)
VIDEO_FILTER_WRAPPER( I420_R5G5B5 )
VIDEO_FILTER_WRAPPER( I420_R5G6B5 )
VIDEO_FILTER_WRAPPER( I420_A8R8G8B8 )
VIDEO_FILTER_WRAPPER( I420_R8G8B8A8 )
VIDEO_FILTER_WRAPPER( I420_B8G8R8A8 )
VIDEO_FILTER_WRAPPER( I420_A8B8G8R8 )
#else
VIDEO_FILTER_WRAPPER( I420_RGB8 )
VIDEO_FILTER_WRAPPER( I420_RGB16 )
VIDEO_FILTER_WRAPPER( I420_RGB32 )




static void SetYUV( filter_t *p_filter, const video_format_t *vfmt )
{
filter_sys_t *p_sys = p_filter->p_sys;
unsigned i_lrshift = ctz(vfmt->i_rmask);
unsigned i_lgshift = ctz(vfmt->i_gmask);
unsigned i_lbshift = ctz(vfmt->i_bmask);
unsigned i_rrshift = 8 - vlc_popcount(vfmt->i_rmask);
unsigned i_rgshift = 8 - vlc_popcount(vfmt->i_gmask);
unsigned i_rbshift = 8 - vlc_popcount(vfmt->i_bmask);






switch( p_filter->fmt_out.video.i_chroma )
{
case VLC_CODEC_RGB8:
p_sys->p_rgb8 = (uint8_t *)p_sys->p_base;
Set8bppPalette( p_filter, p_sys->p_rgb8 );
break;

case VLC_CODEC_RGB15:
case VLC_CODEC_RGB16:
p_sys->p_rgb16 = (uint16_t *)p_sys->p_base;
for( unsigned i_index = 0; i_index < RED_MARGIN; i_index++ )
{
p_sys->p_rgb16[RED_OFFSET - RED_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb16[RED_OFFSET + 256 + i_index] = RGB2PIXEL( p_filter, 255, 0, 0 );
}
for( unsigned i_index = 0; i_index < GREEN_MARGIN; i_index++ )
{
p_sys->p_rgb16[GREEN_OFFSET - GREEN_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb16[GREEN_OFFSET + 256 + i_index] = RGB2PIXEL( p_filter, 0, 255, 0 );
}
for( unsigned i_index = 0; i_index < BLUE_MARGIN; i_index++ )
{
p_sys->p_rgb16[BLUE_OFFSET - BLUE_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb16[BLUE_OFFSET + BLUE_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 255 );
}
for( unsigned i_index = 0; i_index < 256; i_index++ )
{
p_sys->p_rgb16[RED_OFFSET + i_index] = RGB2PIXEL( p_filter, i_index, 0, 0 );
p_sys->p_rgb16[GREEN_OFFSET + i_index] = RGB2PIXEL( p_filter, 0, i_index, 0 );
p_sys->p_rgb16[BLUE_OFFSET + i_index] = RGB2PIXEL( p_filter, 0, 0, i_index );
}
break;

case VLC_CODEC_RGB24:
case VLC_CODEC_RGB32:
p_sys->p_rgb32 = (uint32_t *)p_sys->p_base;
for( unsigned i_index = 0; i_index < RED_MARGIN; i_index++ )
{
p_sys->p_rgb32[RED_OFFSET - RED_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb32[RED_OFFSET + 256 + i_index] = RGB2PIXEL( p_filter, 255, 0, 0 );
}
for( unsigned i_index = 0; i_index < GREEN_MARGIN; i_index++ )
{
p_sys->p_rgb32[GREEN_OFFSET - GREEN_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb32[GREEN_OFFSET + 256 + i_index] = RGB2PIXEL( p_filter, 0, 255, 0 );
}
for( unsigned i_index = 0; i_index < BLUE_MARGIN; i_index++ )
{
p_sys->p_rgb32[BLUE_OFFSET - BLUE_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 0 );
p_sys->p_rgb32[BLUE_OFFSET + BLUE_MARGIN + i_index] = RGB2PIXEL( p_filter, 0, 0, 255 );
}
for( unsigned i_index = 0; i_index < 256; i_index++ )
{
p_sys->p_rgb32[RED_OFFSET + i_index] = RGB2PIXEL( p_filter, i_index, 0, 0 );
p_sys->p_rgb32[GREEN_OFFSET + i_index] = RGB2PIXEL( p_filter, 0, i_index, 0 );
p_sys->p_rgb32[BLUE_OFFSET + i_index] = RGB2PIXEL( p_filter, 0, 0, i_index );
}
break;
}
}

static void Set8bppPalette( filter_t *p_filter, uint8_t *p_rgb8 )
{
#define CLIP( x ) ( ((x < 0) ? 0 : (x > 255) ? 255 : x) << 8 )
filter_sys_t *p_sys = p_filter->p_sys;

int y,u,v;
int r,g,b;
int i = 0, j = 0;
uint16_t *p_cmap_r = p_sys->p_rgb_r;
uint16_t *p_cmap_g = p_sys->p_rgb_g;
uint16_t *p_cmap_b = p_sys->p_rgb_b;

unsigned char p_lookup[PALETTE_TABLE_SIZE];


for ( y = 0; y <= 256; y += 16, i += 128 - 81 )
{
for ( u = 0; u <= 256; u += 32 )
{
for ( v = 0; v <= 256; v += 32 )
{
r = y + ( (V_RED_COEF*(v-128)) >> SHIFT );
g = y + ( (U_GREEN_COEF*(u-128)
+ V_GREEN_COEF*(v-128)) >> SHIFT );
b = y + ( (U_BLUE_COEF*(u-128)) >> SHIFT );

if( r >= 0x00 && g >= 0x00 && b >= 0x00
&& r <= 0xff && g <= 0xff && b <= 0xff )
{


if( j == 256 )
{
msg_Err( p_filter, "no colors left in palette" );
break;
}


p_cmap_r[ j ] = CLIP( r );
p_cmap_g[ j ] = CLIP( g );
p_cmap_b[ j ] = CLIP( b );

#if 0
printf("+++Alloc RGB cmap %d (%d, %d, %d)\n", j,
p_cmap_r[ j ] >>8, p_cmap_g[ j ] >>8,
p_cmap_b[ j ] >>8);
#endif


p_lookup[ i ] = 1;
p_rgb8[ i++ ] = j;
j++;
}
else
{
p_lookup[ i ] = 0;
p_rgb8[ i++ ] = 0;
}
}
}
}





#if 0


p_vout->i_white_pixel = 0xff;
p_vout->i_black_pixel = 0x00;
p_vout->i_gray_pixel = 0x44;
p_vout->i_blue_pixel = 0x3b;
#endif


for ( i = 0, y = 0; y <= 256; y += 16, i += 128 - 81 )
{
for ( u = 0; u <= 256; u += 32 )
{
for ( v = 0; v <= 256; v += 32, i++ )
{
int u2, v2, dist, mindist = 100000000;

if( p_lookup[ i ] || y == 0 )
{
continue;
}


for( u2 = 0; u2 <= 256; u2 += 32 )
{
for( v2 = 0; v2 <= 256; v2 += 32 )
{
j = ((y>>4)<<7) + (u2>>5)*9 + (v2>>5);
dist = (u-u2)*(u-u2) + (v-v2)*(v-v2);


if( p_lookup[ j ] && dist < mindist )
{
p_rgb8[ i ] = p_rgb8[ j ];
mindist = dist;
}

j -= 128;


if( p_lookup[ j ] && dist + 128 < mindist )
{
p_rgb8[ i ] = p_rgb8[ j ];
mindist = dist + 128;
}
}
}
}
}
}
}
#endif
