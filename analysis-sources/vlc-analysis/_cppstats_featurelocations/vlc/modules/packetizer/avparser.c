

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_block.h>

#include "../codec/avcodec/avcodec.h"
#include "../codec/avcodec/avcommon.h"

#include "avparser.h"



#if !defined(MERGE_FFMPEG)
vlc_module_begin ()
AVPARSER_MODULE
vlc_module_end ()
#endif




typedef struct
{
AVCodecParserContext * p_parser_ctx;
AVCodecContext * p_codec_ctx;
int i_offset;
} decoder_sys_t;

static block_t * Packetize( decoder_t *, block_t ** );
static block_t * PacketizeClosed( decoder_t *, block_t ** );




static void FlushPacketizer( decoder_t *p_dec )
{
avparser_ClosePacketizer( VLC_OBJECT( p_dec ) );
p_dec->p_sys = NULL;
int res = avparser_OpenPacketizer( VLC_OBJECT( p_dec ) );
if ( res != VLC_SUCCESS )
{
msg_Err( p_dec, "failed to flush with error %d", res );
p_dec->pf_packetize = PacketizeClosed;
}
}







int avparser_OpenPacketizer( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;


if( p_dec->fmt_in.i_codec != VLC_CODEC_VP9 )
return VLC_EGENERIC;

unsigned i_avcodec_id;

if( !GetFfmpegCodec( p_dec->fmt_in.i_cat, p_dec->fmt_in.i_codec,
&i_avcodec_id, NULL ) )
return VLC_EGENERIC;


vlc_init_avcodec(p_this);


AVCodecParserContext * p_ctx = av_parser_init( i_avcodec_id );
if( !p_ctx )
return VLC_EGENERIC;

AVCodec * p_codec = avcodec_find_decoder( i_avcodec_id );
if( unlikely( !p_codec ) )
{
av_parser_close( p_ctx );
return VLC_EGENERIC;
}

AVCodecContext * p_codec_ctx = avcodec_alloc_context3( p_codec );
if( unlikely( !p_codec_ctx ) )
{
av_parser_close( p_ctx );
return VLC_ENOMEM;
}

p_dec->p_sys = p_sys = malloc( sizeof(*p_sys) );

if( unlikely( !p_sys ) )
{
avcodec_free_context( &p_codec_ctx );
av_parser_close( p_ctx );
return VLC_ENOMEM;
}
p_dec->pf_packetize = Packetize;
p_dec->pf_flush = FlushPacketizer;
p_dec->pf_get_cc = NULL;
p_sys->p_parser_ctx = p_ctx;
p_sys->p_codec_ctx = p_codec_ctx;
p_sys->i_offset = 0;
es_format_Copy( &p_dec->fmt_out, &p_dec->fmt_in );

return VLC_SUCCESS;
}




void avparser_ClosePacketizer( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
if (likely( p_sys != NULL ))
{
avcodec_free_context( &p_sys->p_codec_ctx );
av_parser_close( p_sys->p_parser_ctx );
free( p_sys );
}
}




static block_t *Packetize ( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;

if( pp_block == NULL || *pp_block == NULL )
return NULL;
if( (*pp_block)->i_flags&(BLOCK_FLAG_CORRUPTED) )
{
block_Release( *pp_block );
return NULL;
}

block_t * p_block = *pp_block;

uint8_t * p_indata = p_block->p_buffer + p_sys->i_offset;
int i_inlen = p_block->i_buffer - p_sys->i_offset;
uint8_t * p_outdata;
int i_outlen;

if( p_sys->i_offset == i_inlen )
goto out;

p_sys->i_offset += av_parser_parse2( p_sys->p_parser_ctx, p_sys->p_codec_ctx,
&p_outdata, &i_outlen, p_indata, i_inlen,
TO_AV_TS(p_block->i_pts), TO_AV_TS(p_block->i_dts), -1);

if( unlikely( i_outlen <= 0 || !p_outdata ) )
goto out;

block_t * p_ret = block_Alloc( i_outlen );

if( unlikely ( !p_ret ) )
goto out;

memcpy( p_ret->p_buffer, p_outdata, i_outlen );
p_ret->i_pts = p_block->i_pts;
p_ret->i_dts = p_block->i_dts;
if( p_sys->p_parser_ctx->key_frame == 1 )
p_ret->i_flags |= BLOCK_FLAG_TYPE_I;

p_block->i_pts = p_block->i_dts = VLC_TICK_INVALID;

return p_ret;

out:
p_sys->i_offset = 0;
block_Release( *pp_block );
*pp_block = NULL;
return NULL;
}




static block_t *PacketizeClosed ( decoder_t *p_dec, block_t **pp_block )
{
(void) p_dec;
if( pp_block != NULL && *pp_block != NULL )
block_Release( *pp_block );
return NULL;
}

