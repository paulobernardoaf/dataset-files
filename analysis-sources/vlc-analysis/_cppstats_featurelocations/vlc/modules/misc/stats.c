






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif







#define kBufferSize 0x500

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_demux.h>


static int DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
picture_t * p_pic = NULL;

if( p_block == NULL ) 
return VLCDEC_SUCCESS;

if( !decoder_UpdateVideoFormat( p_dec ) )
p_pic = decoder_NewPicture( p_dec );
if( !p_pic )
goto error;

if( p_block->i_buffer == kBufferSize )
{
msg_Dbg( p_dec, "got %"PRIu64" ms",
MS_FROM_VLC_TICK(*(vlc_tick_t *)p_block->p_buffer) );
msg_Dbg( p_dec, "got %"PRIu64" ms offset",
MS_FROM_VLC_TICK(vlc_tick_now() - *(vlc_tick_t *)p_block->p_buffer) );
*(vlc_tick_t *)(p_pic->p->p_pixels) = *(vlc_tick_t *)p_block->p_buffer;
}
else
{
msg_Dbg( p_dec, "got a packet not from stats demuxer" );
*(vlc_tick_t *)(p_pic->p->p_pixels) = vlc_tick_now();
}

p_pic->date = p_block->i_pts != VLC_TICK_INVALID ?
p_block->i_pts : p_block->i_dts;
p_pic->b_force = true;

error:
block_Release( p_block );
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}

static int OpenDecoder ( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;

msg_Dbg( p_this, "opening stats decoder" );


p_dec->pf_decode = DecodeBlock;


p_dec->fmt_out.i_codec = VLC_CODEC_I420;
p_dec->fmt_out.video.i_width = 100;
p_dec->fmt_out.video.i_height = 100;
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;

return VLC_SUCCESS;
}


#if defined(ENABLE_SOUT)
static block_t *EncodeVideo( encoder_t *p_enc, picture_t *p_pict )
{
(void)p_pict;
block_t * p_block = block_Alloc( kBufferSize );

*(vlc_tick_t*)p_block->p_buffer = vlc_tick_now();
p_block->i_buffer = kBufferSize;
p_block->i_length = kBufferSize;
p_block->i_dts = p_pict->date;

msg_Dbg( p_enc, "putting %"PRIu64"ms",
MS_FROM_VLC_TICK(*(vlc_tick_t*)p_block->p_buffer) );
return p_block;
}

static block_t *EncodeAudio( encoder_t *p_enc, block_t *p_abuff )
{
(void)p_abuff;
(void)p_enc;
return NULL;
}

static int OpenEncoder ( vlc_object_t *p_this )
{
encoder_t *p_enc = (encoder_t *)p_this;

msg_Dbg( p_this, "opening stats encoder" );

p_enc->pf_encode_video = EncodeVideo;
p_enc->pf_encode_audio = EncodeAudio;

return VLC_SUCCESS;
}
#endif


typedef struct
{
es_format_t fmt;
es_out_id_t *p_es;

date_t pts;
} demux_sys_t;

static int Demux( demux_t *p_demux )
{
demux_sys_t *p_sys = p_demux->p_sys;

block_t * p_block = vlc_stream_Block( p_demux->s, kBufferSize );

if( !p_block ) return 1;

p_block->i_dts = p_block->i_pts = date_Increment( &p_sys->pts, kBufferSize );

msg_Dbg( p_demux, "demux got %"PRId64" ms offset",
MS_FROM_VLC_TICK(vlc_tick_now() - *(vlc_tick_t *)p_block->p_buffer) );



es_out_Send( p_demux->out, p_sys->p_es, p_block );

return 1;
}

static int DemuxControl( demux_t *p_demux, int i_query, va_list args )
{
return demux_vaControlHelper( p_demux->s,
0, 0, 0, 1,
i_query, args );
}

static int OpenDemux ( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t*)p_this;
demux_sys_t *p_sys;

p_demux->p_sys = NULL;


if( *p_demux->psz_name == '\0' )
return VLC_EGENERIC;

msg_Dbg( p_demux, "Init Stat demux" );

p_demux->pf_demux = Demux;
p_demux->pf_control = DemuxControl;

p_demux->p_sys = p_sys = malloc( sizeof( demux_sys_t ) );
if( !p_demux->p_sys )
return VLC_ENOMEM;

date_Init( &p_sys->pts, 1, 1 );
date_Set( &p_sys->pts, VLC_TICK_0 );

es_format_Init( &p_sys->fmt, VIDEO_ES, VLC_FOURCC('s','t','a','t') );
p_sys->fmt.video.i_width = 720;
p_sys->fmt.video.i_height= 480;

p_sys->p_es = es_out_Add( p_demux->out, &p_sys->fmt );

return VLC_SUCCESS;
}

static void CloseDemux ( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t*)p_this;

msg_Dbg( p_demux, "Closing Stat demux" );

free( p_demux->p_sys );
}

vlc_module_begin ()
set_shortname( N_("Stats"))
#if defined(ENABLE_SOUT)
set_description( N_("Stats encoder function") )
set_capability( "encoder", 0 )
add_shortcut( "stats" )
set_callback( OpenEncoder )
add_submodule ()
#endif
set_section( N_( "Stats decoder" ), NULL )
set_description( N_("Stats decoder function") )
set_capability( "video decoder", 0 )
add_shortcut( "stats" )
set_callback( OpenDecoder )
add_submodule()
set_section( N_( "Stats decoder" ), NULL )
set_description( N_("Stats decoder function") )
set_capability( "audio decoder", 0 )
add_shortcut( "stats" )
set_callback( OpenDecoder )
add_submodule()
set_section( N_( "Stats decoder" ), NULL )
set_description( N_("Stats decoder function") )
set_capability( "spu decoder", 0 )
add_shortcut( "stats" )
set_callback( OpenDecoder )
add_submodule ()
set_section( N_( "Stats demux" ), NULL )
set_description( N_("Stats demux function") )
set_capability( "demux", 0 )
add_shortcut( "stats" )
set_callbacks( OpenDemux, CloseDemux )
vlc_module_end ()
