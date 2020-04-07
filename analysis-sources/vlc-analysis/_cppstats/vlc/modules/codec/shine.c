#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_block.h>
#include <vlc_block_helper.h>
#include <vlc_bits.h>
#include <vlc_aout.h>
#include <assert.h>
#include <inttypes.h>
#include <shine/layer3.h>
typedef struct
{
shine_t s;
unsigned int samples_per_frame;
block_fifo_t *p_fifo;
unsigned int i_buffer;
uint8_t *p_buffer;
} encoder_sys_t;
static int OpenEncoder ( vlc_object_t * );
static void CloseEncoder ( vlc_object_t * );
static block_t *EncodeFrame ( encoder_t *, block_t * );
vlc_module_begin();
set_category( CAT_INPUT );
set_subcategory( SUBCAT_INPUT_ACODEC );
set_description( N_("MP3 fixed point audio encoder") );
set_capability( "encoder", 50 );
set_callbacks( OpenEncoder, CloseEncoder );
vlc_module_end();
static struct
{
bool busy;
vlc_mutex_t lock;
} entrant = { false, VLC_STATIC_MUTEX, };
static int OpenEncoder( vlc_object_t *p_this )
{
encoder_t *p_enc = (encoder_t*)p_this;
encoder_sys_t *p_sys;
if( (p_enc->fmt_out.i_codec != VLC_CODEC_MP3 && p_enc->fmt_out.i_codec != VLC_CODEC_MPGA) ||
p_enc->fmt_out.audio.i_channels > 2 )
return VLC_EGENERIC;
if( p_enc->fmt_in.audio.i_channels != 2 )
{
msg_Err( p_enc, "Only stereo input is accepted, rejecting %d channels",
p_enc->fmt_in.audio.i_channels );
return VLC_EGENERIC;
}
if( p_enc->fmt_out.i_bitrate <= 0 )
{
msg_Err( p_enc, "unknown bitrate" );
return VLC_EGENERIC;
}
msg_Dbg( p_enc, "bitrate %d, samplerate %d, channels %d",
p_enc->fmt_out.i_bitrate, p_enc->fmt_out.audio.i_rate,
p_enc->fmt_out.audio.i_channels );
vlc_mutex_lock( &entrant.lock );
if( entrant.busy )
{
msg_Err( p_enc, "encoder already in progress" );
vlc_mutex_unlock( &entrant.lock );
return VLC_EGENERIC;
}
entrant.busy = true;
vlc_mutex_unlock( &entrant.lock );
p_enc->p_sys = p_sys = calloc( 1, sizeof( *p_sys ) );
if( !p_sys )
goto enomem;
if( !( p_sys->p_fifo = block_FifoNew() ) )
{
free( p_sys );
goto enomem;
}
shine_config_t cfg = {
.wave = {
.channels = p_enc->fmt_out.audio.i_channels,
.samplerate = p_enc->fmt_out.audio.i_rate,
},
};
shine_set_config_mpeg_defaults(&cfg.mpeg);
cfg.mpeg.bitr = p_enc->fmt_out.i_bitrate / 1000;
if (shine_check_config(cfg.wave.samplerate, cfg.mpeg.bitr) == -1) {
msg_Err(p_enc, "Invalid bitrate %d\n", cfg.mpeg.bitr);
free(p_sys);
return VLC_EGENERIC;
}
p_sys->s = shine_initialise(&cfg);
p_sys->samples_per_frame = shine_samples_per_pass(p_sys->s);
p_enc->pf_encode_audio = EncodeFrame;
p_enc->fmt_out.i_cat = AUDIO_ES;
p_enc->fmt_in.i_codec = VLC_CODEC_S16N;
return VLC_SUCCESS;
enomem:
vlc_mutex_lock( &entrant.lock );
entrant.busy = false;
vlc_mutex_unlock( &entrant.lock );
return VLC_ENOMEM;
}
static block_t *GetPCM( encoder_t *p_enc, block_t *p_block )
{
encoder_sys_t *p_sys = p_enc->p_sys;
block_t *p_pcm_block;
if( !p_block ) goto buffered; 
while( p_sys->i_buffer + p_block->i_buffer >= p_sys->samples_per_frame * 4 )
{
unsigned int i_buffer = 0;
p_pcm_block = block_Alloc( p_sys->samples_per_frame * 4 );
if( !p_pcm_block )
break;
if( p_sys->i_buffer )
{
memcpy( p_pcm_block->p_buffer, p_sys->p_buffer, p_sys->i_buffer );
i_buffer = p_sys->i_buffer;
p_sys->i_buffer = 0;
free( p_sys->p_buffer );
p_sys->p_buffer = NULL;
}
memcpy( p_pcm_block->p_buffer + i_buffer,
p_block->p_buffer, p_sys->samples_per_frame * 4 - i_buffer );
p_block->p_buffer += p_sys->samples_per_frame * 4 - i_buffer;
p_block->i_buffer -= p_sys->samples_per_frame * 4 - i_buffer;
block_FifoPut( p_sys->p_fifo, p_pcm_block );
}
if( p_block->i_buffer )
{
uint8_t *p_tmp;
if( p_sys->i_buffer > 0 )
p_tmp = realloc( p_sys->p_buffer, p_block->i_buffer + p_sys->i_buffer );
else
p_tmp = malloc( p_block->i_buffer );
if( !p_tmp )
{
p_sys->i_buffer = 0;
free( p_sys->p_buffer );
p_sys->p_buffer = NULL;
return NULL;
}
p_sys->p_buffer = p_tmp;
memcpy( p_sys->p_buffer + p_sys->i_buffer,
p_block->p_buffer, p_block->i_buffer );
p_sys->i_buffer += p_block->i_buffer;
p_block->i_buffer = 0;
}
buffered:
return block_FifoCount( p_sys->p_fifo ) > 0 ? block_FifoGet( p_sys->p_fifo ) : NULL;
}
static block_t *EncodeFrame( encoder_t *p_enc, block_t *p_block )
{
if (!p_block) 
return NULL;
encoder_sys_t *p_sys = p_enc->p_sys;
block_t *p_pcm_block;
block_t *p_chain = NULL;
unsigned int i_samples = p_block->i_buffer >> 2 ;
vlc_tick_t start_date = p_block->i_pts;
start_date -= vlc_tick_from_samples(i_samples, p_enc->fmt_out.audio.i_rate);
VLC_UNUSED(p_enc);
do {
p_pcm_block = GetPCM( p_enc, p_block );
if( !p_pcm_block )
break;
p_block = NULL; 
int16_t pcm_planar_buf[SHINE_MAX_SAMPLES * 2];
int16_t *pcm_planar_buf_chans[2] = {
&pcm_planar_buf[0],
&pcm_planar_buf[p_sys->samples_per_frame],
};
aout_Deinterleave( pcm_planar_buf, p_pcm_block->p_buffer,
p_sys->samples_per_frame, p_enc->fmt_in.audio.i_channels, p_enc->fmt_in.i_codec);
int written;
unsigned char *buf = shine_encode_buffer(p_sys->s, pcm_planar_buf_chans, &written);
block_Release( p_pcm_block );
if (written <= 0)
break;
block_t *p_mp3_block = block_Alloc( written );
if( !p_mp3_block )
break;
memcpy( p_mp3_block->p_buffer, buf, written );
p_mp3_block->i_length = vlc_tick_from_samples(p_sys->samples_per_frame,
p_enc->fmt_out.audio.i_rate);
start_date += p_mp3_block->i_length;
p_mp3_block->i_dts = p_mp3_block->i_pts = start_date;
p_mp3_block->i_nb_samples = p_sys->samples_per_frame;
block_ChainAppend( &p_chain, p_mp3_block );
} while( p_pcm_block );
return p_chain;
}
static void CloseEncoder( vlc_object_t *p_this )
{
encoder_sys_t *p_sys = ((encoder_t*)p_this)->p_sys;
vlc_mutex_lock( &entrant.lock );
entrant.busy = false;
vlc_mutex_unlock( &entrant.lock );
if( p_sys->i_buffer )
free( p_sys->p_buffer );
shine_close(p_sys->s);
block_FifoRelease( p_sys->p_fifo );
free( p_sys );
}
