


























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_block_helper.h>
#include <vlc_modules.h>

#include "a52.h"

#include "packetizer_helper.h"

static int Open( vlc_object_t * );
static void Close( vlc_object_t * );

vlc_module_begin ()
set_category(CAT_SOUT)
set_subcategory(SUBCAT_SOUT_PACKETIZER)
set_description( N_("A/52 audio packetizer") )
set_capability( "packetizer", 10 )
set_callbacks( Open, Close )
vlc_module_end ()

typedef struct
{



int i_state;

block_bytestream_t bytestream;




date_t end_date;
vlc_tick_t i_prev_bytestream_pts;
bool b_discontuinity;

vlc_a52_header_t frame;
size_t i_input_size;
} decoder_sys_t;

static void PacketizeFlush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;

p_sys->b_discontuinity = true;
date_Set( &p_sys->end_date, VLC_TICK_INVALID );
p_sys->i_state = STATE_NOSYNC;
p_sys->i_prev_bytestream_pts = VLC_TICK_INVALID;
block_BytestreamEmpty( &p_sys->bytestream );
}

static block_t *GetOutBuffer( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;

assert( p_sys->frame.i_rate > 0 );

block_t *p_block = block_Alloc( p_sys->i_input_size );
if( p_block == NULL )
return NULL;

if( p_dec->fmt_out.audio.i_rate != p_sys->frame.i_rate )
{
msg_Dbg( p_dec, "A/52 channels:%d samplerate:%d bitrate:%d",
p_sys->frame.i_channels, p_sys->frame.i_rate, p_sys->frame.i_bitrate );
if( p_sys->end_date.i_divider_num )
date_Change( &p_sys->end_date, p_sys->frame.i_rate, 1 );
else
date_Init( &p_sys->end_date, p_sys->frame.i_rate, 1 );
}

if( p_sys->bytestream.p_block->i_pts != date_Get( &p_sys->end_date ) &&
p_sys->bytestream.p_block->i_pts != VLC_TICK_INVALID )
{


if( p_sys->bytestream.p_block->i_pts != p_sys->i_prev_bytestream_pts )
date_Set( &p_sys->end_date, p_sys->bytestream.p_block->i_pts );
p_sys->i_prev_bytestream_pts = p_sys->bytestream.p_block->i_pts;
p_sys->bytestream.p_block->i_pts = VLC_TICK_INVALID;
}

p_dec->fmt_out.audio.i_rate = p_sys->frame.i_rate;
p_dec->fmt_out.audio.i_channels = p_sys->frame.i_channels;
if( p_dec->fmt_out.audio.i_bytes_per_frame < p_sys->frame.i_size )
p_dec->fmt_out.audio.i_bytes_per_frame = p_sys->frame.i_size;
p_dec->fmt_out.audio.i_frame_length = p_sys->frame.i_samples;

p_dec->fmt_out.audio.i_chan_mode = p_sys->frame.i_chan_mode;
p_dec->fmt_out.audio.i_physical_channels = p_sys->frame.i_channels_conf;

p_dec->fmt_out.i_bitrate = p_sys->frame.i_bitrate;

p_block->i_nb_samples = p_sys->frame.i_samples;
p_block->i_pts = p_block->i_dts = date_Get( &p_sys->end_date );
if( p_block->i_pts != VLC_TICK_INVALID )
p_block->i_length = date_Increment( &p_sys->end_date,
p_block->i_nb_samples ) - p_block->i_pts;

return p_block;
}

static block_t *PacketizeBlock( decoder_t *p_dec, block_t **pp_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t p_header[VLC_A52_MIN_HEADER_SIZE];
block_t *p_out_buffer;

block_t *p_block = pp_block ? *pp_block : NULL;

if( p_block )
{
if( p_block->i_flags & (BLOCK_FLAG_DISCONTINUITY | BLOCK_FLAG_CORRUPTED) )
{

block_t *p_drain = PacketizeBlock( p_dec, NULL );
if(p_drain)
return p_drain;

PacketizeFlush( p_dec );

if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release( p_block );
return NULL;
}
}

block_BytestreamPush( &p_sys->bytestream, p_block );
}

while( 1 )
{
switch( p_sys->i_state )
{
case STATE_NOSYNC:
while( block_PeekBytes( &p_sys->bytestream, p_header, 2 )
== VLC_SUCCESS )
{
if( p_header[0] == 0x0b && p_header[1] == 0x77 )
{
p_sys->i_state = STATE_SYNC;
break;
}
block_SkipByte( &p_sys->bytestream );
}
if( p_sys->i_state != STATE_SYNC )
{
block_BytestreamFlush( &p_sys->bytestream );


return NULL;
}


case STATE_SYNC:
p_sys->i_state = STATE_HEADER;


case STATE_HEADER:

if( block_PeekBytes( &p_sys->bytestream, p_header,
VLC_A52_MIN_HEADER_SIZE ) != VLC_SUCCESS )
{

return NULL;
}


if( vlc_a52_header_Parse( &p_sys->frame, p_header,
VLC_A52_MIN_HEADER_SIZE ) != VLC_SUCCESS )
{
msg_Dbg( p_dec, "emulated sync word" );
block_SkipByte( &p_sys->bytestream );
p_sys->i_state = STATE_NOSYNC;
break;
}

if( p_sys->frame.b_eac3 && p_sys->frame.bs.eac3.strmtyp == EAC3_STRMTYP_DEPENDENT )
{
msg_Warn( p_dec, "starting with dependent stream, skip it" );
p_sys->i_state = STATE_NOSYNC;
if( block_SkipBytes( &p_sys->bytestream,
p_sys->frame.i_size ) != VLC_SUCCESS )
return NULL;
break;
}

p_sys->i_input_size = p_sys->frame.i_size;
p_sys->i_state = STATE_NEXT_SYNC;


case STATE_NEXT_SYNC:

if( block_PeekOffsetBytes( &p_sys->bytestream, p_sys->i_input_size,
p_header, VLC_A52_MIN_HEADER_SIZE )
!= VLC_SUCCESS )
{
if( p_block == NULL ) 
{
p_sys->i_state = STATE_GET_DATA;
break;
}

return NULL;
}

if( p_header[0] == 0 || p_header[1] == 0 )
{

p_sys->i_state = STATE_GET_DATA;
break;
}

if( p_header[0] != 0x0b || p_header[1] != 0x77 )
{
msg_Dbg( p_dec, "emulated sync word "
"(no sync on following frame)" );
p_sys->i_state = STATE_NOSYNC;
block_SkipByte( &p_sys->bytestream );
break;
}

vlc_a52_header_t a52;
if( !vlc_a52_header_Parse( &a52, p_header, VLC_A52_MIN_HEADER_SIZE )
&& a52.b_eac3 && a52.bs.eac3.strmtyp == EAC3_STRMTYP_DEPENDENT )
{
p_sys->i_input_size += a52.i_size;
p_dec->fmt_out.i_codec = VLC_CODEC_A52;
p_dec->fmt_out.i_profile = VLC_A52_PROFILE_EAC3_DEPENDENT;
}

p_sys->i_state = STATE_GET_DATA;
break;

case STATE_GET_DATA:

if( block_WaitBytes( &p_sys->bytestream,
p_sys->i_input_size ) != VLC_SUCCESS )
{

return NULL;
}
p_sys->i_state = STATE_SEND_DATA;


case STATE_SEND_DATA:
if( !(p_out_buffer = GetOutBuffer( p_dec )) )
{
return NULL;
}



block_GetBytes( &p_sys->bytestream, p_out_buffer->p_buffer,
p_out_buffer->i_buffer );

p_sys->i_state = STATE_NOSYNC;

if( p_out_buffer->i_dts == VLC_TICK_INVALID )
{
block_Release( p_out_buffer );
return NULL;
}

if( p_sys->b_discontuinity )
{
p_out_buffer->i_flags |= BLOCK_FLAG_DISCONTINUITY;
p_sys->b_discontuinity = false;
}


if( pp_block )
*pp_block = block_BytestreamPop( &p_sys->bytestream );

return p_out_buffer;
}
}
}

static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;

block_BytestreamRelease( &p_sys->bytestream );

free( p_sys );
}

static int Open( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;

switch( p_dec->fmt_in.i_codec )
{
case VLC_CODEC_EAC3:
case VLC_CODEC_A52:
break;
default:
return VLC_EGENERIC;
}


if( ( p_dec->p_sys = p_sys =
(decoder_sys_t *)malloc(sizeof(decoder_sys_t)) ) == NULL )
return VLC_ENOMEM;


p_sys->i_state = STATE_NOSYNC;
date_Set( &p_sys->end_date, VLC_TICK_INVALID );
p_sys->b_discontuinity = false;
p_sys->i_prev_bytestream_pts = VLC_TICK_INVALID;
memset(&p_sys->frame, 0, sizeof(vlc_a52_header_t));

block_BytestreamInit( &p_sys->bytestream );


p_dec->fmt_out.i_codec = p_dec->fmt_in.i_codec;
p_dec->fmt_out.audio = p_dec->fmt_in.audio;
p_dec->fmt_out.audio.i_rate = 0;


p_dec->pf_packetize = PacketizeBlock;
p_dec->pf_flush = PacketizeFlush;
p_dec->pf_get_cc = NULL;
return VLC_SUCCESS;
}
