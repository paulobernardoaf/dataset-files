#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <libbpg.h>
typedef struct
{
struct BPGDecoderContext *p_bpg;
} decoder_sys_t;
static int OpenDecoder(vlc_object_t *);
static void CloseDecoder(vlc_object_t *);
static int DecodeBlock(decoder_t *, block_t *);
vlc_module_begin()
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_description( N_("BPG image decoder") )
set_capability( "video decoder", 60 )
set_callbacks( OpenDecoder, CloseDecoder )
add_shortcut( "bpg" )
vlc_module_end()
static int OpenDecoder(vlc_object_t *p_this)
{
decoder_t *p_dec = (decoder_t *)p_this;
if( p_dec->fmt_in.i_codec != VLC_CODEC_BPG )
{
return VLC_EGENERIC;
}
decoder_sys_t *p_sys = malloc( sizeof( decoder_sys_t ) );
if( p_sys == NULL )
{
return VLC_ENOMEM;
}
p_dec->p_sys = p_sys;
p_sys->p_bpg = bpg_decoder_open();
if( !p_sys->p_bpg )
{
return VLC_EGENERIC;
}
p_dec->pf_decode = DecodeBlock;
return VLC_SUCCESS;
}
static int DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic = 0;
BPGImageInfo img_info;
if( p_block == NULL ) 
return VLCDEC_SUCCESS;
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
goto error;
if( bpg_decoder_decode( p_sys->p_bpg,
p_block->p_buffer,
p_block->i_buffer ) < 0 )
{
msg_Err( p_dec, "Could not decode block" );
goto error;
}
if( bpg_decoder_get_info( p_sys->p_bpg, &img_info ) )
{
msg_Err( p_dec, "Could not get info for decoder" );
goto error;
}
if( bpg_decoder_start( p_sys->p_bpg, BPG_OUTPUT_FORMAT_RGB24 ) )
{
msg_Err( p_dec, "Could not start decoder" );
goto error;
}
p_dec->fmt_out.video.i_chroma =
p_dec->fmt_out.i_codec = VLC_CODEC_RGB24;
video_format_FixRgb(&p_dec->fmt_out.video);
p_dec->fmt_out.video.i_visible_width = p_dec->fmt_out.video.i_width = img_info.width;
p_dec->fmt_out.video.i_visible_height = p_dec->fmt_out.video.i_height = img_info.height;
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;
if( decoder_UpdateVideoFormat( p_dec ) )
{
goto error;
}
p_pic = decoder_NewPicture( p_dec );
if( !p_pic )
{
goto error;
}
const int img_height = img_info.height;
for (int i = 0; i < img_height; i++)
{
if( bpg_decoder_get_line( p_sys->p_bpg,
p_pic->p->p_pixels + p_pic->p->i_pitch * i )
< 0 )
{
msg_Err( p_dec, "Could not decode line" );
goto error;
}
}
p_pic->date = p_block->i_pts != VLC_TICK_INVALID ? p_block->i_pts : p_block->i_dts;
decoder_QueueVideo( p_dec, p_pic );
error:
block_Release( p_block );
return VLCDEC_SUCCESS;
}
static void CloseDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_sys->p_bpg )
bpg_decoder_close( p_sys->p_bpg );
free( p_sys );
}
