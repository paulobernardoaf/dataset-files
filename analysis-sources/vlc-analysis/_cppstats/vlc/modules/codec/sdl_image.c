#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <SDL_image.h>
typedef struct
{
const char *psz_sdl_type;
} decoder_sys_t;
static int OpenDecoder ( vlc_object_t * );
static void CloseDecoder ( vlc_object_t * );
static int DecodeBlock ( decoder_t *, block_t * );
vlc_module_begin ()
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_shortname( N_("SDL Image decoder"))
set_description( N_("SDL_image video decoder") )
set_capability( "video decoder", 60 )
set_callbacks( OpenDecoder, CloseDecoder )
add_shortcut( "sdl_image" )
vlc_module_end ()
static const struct supported_fmt_t
{
vlc_fourcc_t i_fourcc;
const char *psz_sdl_type;
} p_supported_fmt[] =
{
{ VLC_CODEC_TARGA, "TGA" },
{ VLC_CODEC_BMP, "BMP" },
{ VLC_CODEC_PNM, "PNM" },
{ VLC_FOURCC('x','p','m',' '), "XPM" },
{ VLC_FOURCC('x','c','f',' '), "XCF" },
{ VLC_CODEC_PCX, "PCX" },
{ VLC_CODEC_GIF, "GIF" },
{ VLC_CODEC_JPEG, "JPG" },
{ VLC_CODEC_TIFF, "TIF" },
{ VLC_FOURCC('l','b','m',' '), "LBM" },
{ VLC_CODEC_PNG, "PNG" }
};
static int OpenDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys;
int i;
for ( i = 0;
i < (int)(sizeof(p_supported_fmt)/sizeof(struct supported_fmt_t));
i++ )
{
if ( p_supported_fmt[i].i_fourcc == p_dec->fmt_in.i_codec )
break;
}
if ( i == (int)(sizeof(p_supported_fmt)/sizeof(struct supported_fmt_t)) )
{
return VLC_EGENERIC;
}
if( ( p_dec->p_sys = p_sys =
(decoder_sys_t *)malloc(sizeof(decoder_sys_t)) ) == NULL )
return VLC_ENOMEM;
p_sys->psz_sdl_type = p_supported_fmt[i].psz_sdl_type;
p_dec->fmt_out.i_codec = VLC_CODEC_RGB32;
p_dec->pf_decode = DecodeBlock;
return VLC_SUCCESS;
}
static int DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic = NULL;
SDL_Surface *p_surface;
SDL_RWops *p_rw;
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
p_rw = SDL_RWFromConstMem( p_block->p_buffer, p_block->i_buffer );
p_surface = IMG_LoadTyped_RW( p_rw, 1, (char*)p_sys->psz_sdl_type );
if ( p_surface == NULL )
{
msg_Warn( p_dec, "SDL_image couldn't load the image (%s)",
IMG_GetError() );
goto error;
}
switch ( p_surface->format->BitsPerPixel )
{
case 16:
p_dec->fmt_out.i_codec = VLC_CODEC_RGB16;
break;
case 8:
case 24:
p_dec->fmt_out.i_codec = VLC_CODEC_RGB24;
break;
case 32:
p_dec->fmt_out.i_codec = VLC_CODEC_RGB32;
break;
default:
msg_Warn( p_dec, "unknown bits/pixel format (%d)",
p_surface->format->BitsPerPixel );
goto error;
}
p_dec->fmt_out.video.i_width = p_surface->w;
p_dec->fmt_out.video.i_height = p_surface->h;
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;
if( decoder_UpdateVideoFormat( p_dec ) )
goto error;
p_pic = decoder_NewPicture( p_dec );
if ( p_pic == NULL ) goto error;
switch ( p_surface->format->BitsPerPixel )
{
case 8:
{
for ( int i = 0; i < p_surface->h; i++ )
{
uint8_t *p_src = (uint8_t*)p_surface->pixels + i * p_surface->pitch;
uint8_t *p_dst = p_pic->p[0].p_pixels + i * p_pic->p[0].i_pitch;
for ( int j = 0; j < p_surface->w; j++ )
{
uint8_t r, g, b;
SDL_GetRGB( *(p_src++), p_surface->format,
&r, &g, &b );
*(p_dst++) = r;
*(p_dst++) = g;
*(p_dst++) = b;
}
}
break;
}
case 16:
{
uint8_t *p_src = p_surface->pixels;
uint8_t *p_dst = p_pic->p[0].p_pixels;
int i_pitch = p_pic->p[0].i_pitch < p_surface->pitch ?
p_pic->p[0].i_pitch : p_surface->pitch;
for ( int i = 0; i < p_surface->h; i++ )
{
memcpy( p_dst, p_src, i_pitch );
p_src += p_surface->pitch;
p_dst += p_pic->p[0].i_pitch;
}
break;
}
case 24:
{
for ( int i = 0; i < p_surface->h; i++ )
{
uint8_t *p_src = (uint8_t*)p_surface->pixels + i * p_surface->pitch;
uint8_t *p_dst = p_pic->p[0].p_pixels + i * p_pic->p[0].i_pitch;
for ( int j = 0; j < p_surface->w; j++ )
{
uint8_t r, g, b;
SDL_GetRGB( *(uint32_t*)p_src, p_surface->format,
&r, &g, &b );
*(p_dst++) = r;
*(p_dst++) = g;
*(p_dst++) = b;
p_src += 3;
}
}
break;
}
case 32:
{
for ( int i = 0; i < p_surface->h; i++ )
{
uint8_t *p_src = (uint8_t*)p_surface->pixels + i * p_surface->pitch;
uint8_t *p_dst = p_pic->p[0].p_pixels + i * p_pic->p[0].i_pitch;
for ( int j = 0; j < p_surface->w; j++ )
{
uint8_t r, g, b, a;
SDL_GetRGBA( *(uint32_t*)p_src, p_surface->format,
&r, &g, &b, &a );
*(p_dst++) = b;
*(p_dst++) = g;
*(p_dst++) = r;
*(p_dst++) = a;
p_src += 4;
}
}
break;
}
}
p_pic->date = (p_block->i_pts != VLC_TICK_INVALID) ?
p_block->i_pts : p_block->i_dts;
decoder_QueueVideo( p_dec, p_pic );
error:
if ( p_surface != NULL ) SDL_FreeSurface( p_surface );
block_Release( p_block );
return VLCDEC_SUCCESS;
}
static void CloseDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
free( p_sys );
}
