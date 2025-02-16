

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>

#include "spudec.h"




static int DecoderOpen ( vlc_object_t * );
static int PacketizerOpen( vlc_object_t * );
static void Close ( vlc_object_t * );

#define DVDSUBTRANS_DISABLE_TEXT N_("Disable DVD subtitle transparency")
#define DVDSUBTRANS_DISABLE_LONGTEXT N_("Removes all transparency effects " "used in DVD subtitles.")


vlc_module_begin ()
set_description( N_("DVD subtitles decoder") )
set_shortname( N_("DVD subtitles") )
set_capability( "spu decoder", 75 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_SCODEC )
set_callbacks( DecoderOpen, Close )

add_bool( "dvdsub-transparency", false,
DVDSUBTRANS_DISABLE_TEXT, DVDSUBTRANS_DISABLE_LONGTEXT, true )
add_submodule ()
set_description( N_("DVD subtitles packetizer") )
set_capability( "packetizer", 50 )
set_callbacks( PacketizerOpen, Close )
vlc_module_end ()




static block_t * Reassemble( decoder_t *, block_t * );
static int Decode ( decoder_t *, block_t * );
static block_t * Packetize ( decoder_t *, block_t ** );

static int OpenCommon( vlc_object_t *p_this, bool b_packetizer )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;

if( p_dec->fmt_in.i_codec != VLC_CODEC_SPU )
return VLC_EGENERIC;

p_dec->p_sys = p_sys = malloc( sizeof( decoder_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;

p_sys->b_packetizer = b_packetizer;
p_sys->b_disabletrans = var_InheritBool( p_dec, "dvdsub-transparency" );
p_sys->i_spu_size = 0;
p_sys->i_spu = 0;
p_sys->p_block = NULL;

if( b_packetizer )
{
p_dec->pf_packetize = Packetize;
es_format_Copy( &p_dec->fmt_out, &p_dec->fmt_in );
p_dec->fmt_out.i_codec = VLC_CODEC_SPU;
}
else
{
p_dec->fmt_out.i_codec = VLC_CODEC_SPU;
p_dec->pf_decode = Decode;
}

return VLC_SUCCESS;
}

static int DecoderOpen( vlc_object_t *p_this )
{
return OpenCommon( p_this, false );
}

static int PacketizerOpen( vlc_object_t *p_this )
{
return OpenCommon( p_this, true );
}




static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;

if( p_sys->p_block )
{
block_ChainRelease( p_sys->p_block );
}

free( p_sys );
}




static int Decode( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t *p_spu_block;

if( p_block == NULL ) 
return VLCDEC_SUCCESS;
p_spu_block = Reassemble( p_dec, p_block );

if( ! p_spu_block )
{
return VLCDEC_SUCCESS;
}


p_sys->i_spu = block_ChainExtract( p_spu_block, p_sys->buffer, 65536 );
p_sys->i_pts = p_spu_block->i_pts;
block_ChainRelease( p_spu_block );


ParsePacket( p_dec, decoder_QueueSub );


p_sys->i_spu_size = 0;
p_sys->i_rle_size = 0;
p_sys->i_spu = 0;
p_sys->p_block = NULL;

return VLCDEC_SUCCESS;
}




static block_t *Packetize( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( pp_block == NULL ) 
return NULL;
block_t *p_block = *pp_block; *pp_block = NULL;
if( p_block == NULL )
return NULL;

block_t *p_spu = Reassemble( p_dec, p_block );

if( ! p_spu )
{
return NULL;
}

p_spu->i_dts = p_spu->i_pts;
p_spu->i_length = VLC_TICK_INVALID;


p_sys->i_spu_size = 0;
p_sys->i_rle_size = 0;
p_sys->i_spu = 0;
p_sys->p_block = NULL;

return block_ChainGather( p_spu );
}




static block_t *Reassemble( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;

if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return NULL;
}

if( p_sys->i_spu_size <= 0 &&
( p_block->i_pts == VLC_TICK_INVALID || p_block->i_buffer < 4 ) )
{
msg_Dbg( p_dec, "invalid starting packet (size < 4 or pts <=0)" );
msg_Dbg( p_dec, "spu size: %d, i_pts: %"PRId64" i_buffer: %zu",
p_sys->i_spu_size, p_block->i_pts, p_block->i_buffer );
block_Release( p_block );
return NULL;
}

block_ChainAppend( &p_sys->p_block, p_block );
p_sys->i_spu += p_block->i_buffer;

if( p_sys->i_spu_size <= 0 )
{
p_sys->i_spu_size = ( p_block->p_buffer[0] << 8 )|
p_block->p_buffer[1];
p_sys->i_rle_size = ( ( p_block->p_buffer[2] << 8 )|
p_block->p_buffer[3] ) - 4;




if( p_sys->i_spu_size <= 0 || p_sys->i_rle_size >= p_sys->i_spu_size )
{
p_sys->i_spu_size = 0;
p_sys->i_rle_size = 0;
p_sys->i_spu = 0;
p_sys->p_block = NULL;

block_Release( p_block );
return NULL;
}
}

if( p_sys->i_spu >= p_sys->i_spu_size )
{

if( p_sys->i_spu > p_sys->i_spu_size )
msg_Dbg( p_dec, "SPU packets size=%d should be %d",
p_sys->i_spu, p_sys->i_spu_size );

return p_sys->p_block;
}
return NULL;
}
