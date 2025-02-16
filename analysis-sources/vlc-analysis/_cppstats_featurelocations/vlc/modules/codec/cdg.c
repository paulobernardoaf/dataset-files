
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>






#define CDG_SCREEN_WIDTH 300u
#define CDG_SCREEN_HEIGHT 216u


#define CDG_SCREEN_BORDER_WIDTH 6u
#define CDG_SCREEN_BORDER_HEIGHT 12u


#define CDG_DISPLAY_WIDTH (CDG_SCREEN_WIDTH-2*CDG_SCREEN_BORDER_WIDTH)
#define CDG_DISPLAY_HEIGHT (CDG_SCREEN_HEIGHT-2*CDG_SCREEN_BORDER_HEIGHT)

#define CDG_SCREEN_PITCH CDG_SCREEN_WIDTH

typedef struct
{
uint8_t color[16][3];
unsigned i_offseth;
unsigned i_offsetv;
uint8_t screen[CDG_SCREEN_PITCH*CDG_SCREEN_HEIGHT];
uint8_t *p_screen;

int i_packet;
} decoder_sys_t;

#define CDG_PACKET_SIZE 24u

#define CDG_COLOR_R_SHIFT 0
#define CDG_COLOR_G_SHIFT 8
#define CDG_COLOR_B_SHIFT 16




static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );

static int Decode( decoder_t *, block_t * );

static int DecodePacket( decoder_sys_t *p_cdg, uint8_t *p_buffer, int i_buffer );
static void Flush( decoder_t * );
static int Render( decoder_sys_t *p_cdg, picture_t *p_picture );




vlc_module_begin ()
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_description( N_("CDG video decoder") )
set_capability( "video decoder", 1000 )
set_callbacks( Open, Close )
add_shortcut( "cdg" )
vlc_module_end ()




static int Open( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;

if( p_dec->fmt_in.i_codec != VLC_CODEC_CDG )
return VLC_EGENERIC;


p_dec->p_sys = p_sys = calloc( 1, sizeof(decoder_sys_t) );
if( !p_sys )
return VLC_ENOMEM;


p_sys->p_screen = p_sys->screen;
p_sys->i_packet = 0;



p_dec->fmt_out.i_codec = VLC_CODEC_RGB32;
p_dec->fmt_out.video.i_width = CDG_DISPLAY_WIDTH;
p_dec->fmt_out.video.i_height = CDG_DISPLAY_HEIGHT;
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;
p_dec->fmt_out.video.i_rmask = 0xff << CDG_COLOR_R_SHIFT;
p_dec->fmt_out.video.i_gmask = 0xff << CDG_COLOR_G_SHIFT;
p_dec->fmt_out.video.i_bmask = 0xff << CDG_COLOR_B_SHIFT;


p_dec->pf_decode = Decode;
p_dec->pf_flush = Flush;

return VLC_SUCCESS;
}




static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;

p_sys->i_packet = 0;
}






static int Decode( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic = NULL;

if( !p_block ) 
return VLCDEC_SUCCESS;

if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
Flush( p_dec );
goto exit;
}


while( p_block->i_buffer >= CDG_PACKET_SIZE )
{
DecodePacket( p_sys, p_block->p_buffer, CDG_PACKET_SIZE );
p_block->i_buffer -= CDG_PACKET_SIZE;
p_block->p_buffer += CDG_PACKET_SIZE;
}


if( (p_sys->i_packet%3) == 1 && p_block->i_pts == p_block->i_dts )
{

if( decoder_UpdateVideoFormat( p_dec ) )
goto exit;
p_pic = decoder_NewPicture( p_dec );
if( !p_pic )
goto exit;

Render( p_sys, p_pic );
p_pic->date = p_block->i_pts != VLC_TICK_INVALID ? p_block->i_pts : p_block->i_dts;
}

exit:
block_Release( p_block );
if( p_pic != NULL )
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}




static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;

free( p_sys );
}




static void ScreenFill( decoder_sys_t *p_cdg, int sx, int sy, int dx, int dy, int c )
{
int x, y;
for( y = sy; y < sy+dy; y++ )
for( x = sx; x < sx+dx; x++ )
p_cdg->p_screen[y*CDG_SCREEN_PITCH+x] = c;
}
static int DecodeMemoryPreset( decoder_sys_t *p_cdg, const uint8_t *p_data )
{
const int i_color = p_data[0]&0x0f;
#if 0
const int i_repeat= p_data[1]&0x0f;
#endif

ScreenFill( p_cdg, 0, 0, CDG_SCREEN_WIDTH, CDG_SCREEN_HEIGHT, i_color );
return 0;
}
static int DecodeBorderPreset( decoder_sys_t *p_cdg, const uint8_t *p_data )
{
const int i_color = p_data[0]&0x0f;


ScreenFill( p_cdg, 0, 0,
CDG_SCREEN_WIDTH, CDG_SCREEN_BORDER_HEIGHT, i_color );
ScreenFill( p_cdg, 0, CDG_SCREEN_HEIGHT-CDG_SCREEN_BORDER_HEIGHT,
CDG_SCREEN_WIDTH, CDG_SCREEN_BORDER_HEIGHT, i_color );
ScreenFill( p_cdg, 0, CDG_SCREEN_BORDER_HEIGHT,
CDG_SCREEN_BORDER_WIDTH, CDG_SCREEN_HEIGHT-CDG_SCREEN_BORDER_HEIGHT, i_color );
ScreenFill( p_cdg, CDG_SCREEN_WIDTH-CDG_SCREEN_BORDER_WIDTH, CDG_SCREEN_BORDER_HEIGHT,
CDG_SCREEN_BORDER_WIDTH, CDG_SCREEN_HEIGHT-CDG_SCREEN_BORDER_HEIGHT, i_color );
return 0;
}

static int DecodeLoadColorTable( decoder_sys_t *p_cdg, const uint8_t *p_data, int i_base )
{
int n;
for( n = 0; n < 8; n++ )
{
const int c = ((p_data[2*n+0] << 8) | p_data[2*n+1]);
const int r = (c >> 10)&0x0f;
const int g = ((c >> 6)&0x0c) | ((c >> 4)&0x03);
const int b = c &0x0f;

p_cdg->color[i_base+n][0] = r << 4;
p_cdg->color[i_base+n][1] = g << 4;
p_cdg->color[i_base+n][2] = b << 4;
}
return 0;
}
static int DecodeTileBlock( decoder_sys_t *p_cdg, const uint8_t *p_data, int doXor )
{
int p_color[2];
int sx, sy;
int x, y;

p_color[0] = p_data[0] & 0x0f;
p_color[1] = p_data[1] & 0x0f;

sy = (p_data[2] & 0x1f)*12;
sx = (p_data[3] & 0x3f)*6;

for( y = 0; y < 12; y++ )
{
for( x = 0; x < 6; x++ )
{
const int idx = ( p_data[4+y] >> (5-x) ) & 0x01;

unsigned index = (sy+y)*CDG_SCREEN_PITCH+(sx+x);
if( index >= CDG_SCREEN_PITCH*CDG_SCREEN_HEIGHT )
return 0;

uint8_t *p = &p_cdg->p_screen[index];

if( doXor )
*p ^= p_color[idx];
else
*p = p_color[idx];
}
}
return 0;
}

static int DecodeScroll( decoder_sys_t *p_cdg, const uint8_t *p_data, int b_copy )
{
uint8_t copy[CDG_SCREEN_PITCH*CDG_SCREEN_HEIGHT];

uint8_t color = p_data[0]&0x0f;
int i_shifth;
int i_shiftv;


p_cdg->i_offseth = p_data[1]&0x7;
if( p_cdg->i_offseth >= CDG_SCREEN_BORDER_WIDTH )
p_cdg->i_offseth = CDG_SCREEN_BORDER_WIDTH-1;

p_cdg->i_offsetv = p_data[2]&0xf;
if( p_cdg->i_offsetv >= CDG_SCREEN_BORDER_HEIGHT )
p_cdg->i_offsetv = CDG_SCREEN_BORDER_HEIGHT-1;


switch( (p_data[1] >> 4)&0x3 )
{
case 0x01: i_shifth = 6; break;
case 0x02: i_shifth = -6; break;
default:
i_shifth = 0;
break;
}
switch( (p_data[2] >> 4)&0x3 )
{
case 0x01: i_shiftv = 12; break;
case 0x02: i_shiftv =-12; break;
default:
i_shiftv = 0;
break;
}

if( i_shifth == 0 && i_shiftv == 0 )
return 0;


memcpy( copy, p_cdg->screen, sizeof(p_cdg->screen) );


ScreenFill( p_cdg, 0, 0, CDG_SCREEN_WIDTH, CDG_SCREEN_HEIGHT, color );


for( unsigned y = 0; y < CDG_SCREEN_HEIGHT; y++ )
{
int dy = i_shiftv + y;
for( unsigned x = 0; x < CDG_SCREEN_WIDTH; x++ )
{
int dx = i_shifth + x;

if( b_copy )
{
dy = (dy + CDG_SCREEN_HEIGHT) % CDG_SCREEN_HEIGHT;
dx = (dx + CDG_SCREEN_WIDTH ) % CDG_SCREEN_WIDTH;
}
else
{
if( dy < 0 || (unsigned)dy >= CDG_SCREEN_HEIGHT ||
dx < 0 || (unsigned)dx >= CDG_SCREEN_WIDTH )
continue;
}
p_cdg->screen[dy*CDG_SCREEN_PITCH+dx] = copy[y*CDG_SCREEN_PITCH+x];
}
}


return 0;
}

static int DecodePacket( decoder_sys_t *p_cdg, uint8_t *p_buffer, int i_buffer )
{
const int i_cmd = p_buffer[0] & 0x3f;
const int i_instruction = p_buffer[1] & 0x3f;
const uint8_t *p_data = &p_buffer[4];

if( i_buffer != CDG_PACKET_SIZE )
return -1;

p_cdg->i_packet++;


if( i_cmd != 0x09 )
return 0;

switch( i_instruction )
{
case 1:
DecodeMemoryPreset( p_cdg, p_data );
break;
case 2:
DecodeBorderPreset( p_cdg, p_data );
break;
case 6:
DecodeTileBlock( p_cdg, p_data, 0 );
break;
case 20:
DecodeScroll( p_cdg, p_data, 0 );
break;
case 24:
DecodeScroll( p_cdg, p_data, 1 );
break;
case 28:



break;
case 30:
DecodeLoadColorTable( p_cdg, p_data, 0 );
break;
case 31:
DecodeLoadColorTable( p_cdg, p_data, 8 );
break;
case 38:
DecodeTileBlock( p_cdg, p_data, 1 );
break;
default:
break;
}
return 0;
}

static void RenderPixel( picture_t *p_picture, int x, int y, uint32_t c )
{
const int i_plane = 0;
const int i_pitch = p_picture->p[i_plane].i_pitch;
uint32_t *s = (uint32_t*)&p_picture->p[i_plane].p_pixels[y*i_pitch + x*sizeof(uint32_t)];
*s = c;
}
static uint32_t RenderRGB( int r, int g, int b )
{
return ( r << CDG_COLOR_R_SHIFT ) | ( g << CDG_COLOR_G_SHIFT ) | ( b << CDG_COLOR_B_SHIFT );
}

static int Render( decoder_sys_t *p_cdg, picture_t *p_picture )
{
for( unsigned y = 0; y < CDG_DISPLAY_HEIGHT; y++ )
{
for( unsigned x = 0; x < CDG_DISPLAY_WIDTH; x++ )
{
const int sx = x + p_cdg->i_offseth + CDG_SCREEN_BORDER_WIDTH;
const int sy = y + p_cdg->i_offsetv + CDG_SCREEN_BORDER_HEIGHT;
uint8_t cidx = p_cdg->p_screen[sy*CDG_SCREEN_PITCH +sx];
uint8_t r = p_cdg->color[cidx][0];
uint8_t g = p_cdg->color[cidx][1];
uint8_t b = p_cdg->color[cidx][2];

RenderPixel( p_picture, x, y, RenderRGB( r, g, b ) );
}
}
return 0;
}

