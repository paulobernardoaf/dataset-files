#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_cpu.h>
#include <stdint.h> 
#if defined(USE_A52DEC_TREE)
#include "include/a52.h"
#else
#include "a52dec/a52.h"
#endif
#include <vlc_aout.h>
#include <vlc_block.h>
#include <vlc_codec.h>
static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
typedef struct
{
a52_state_t *p_liba52; 
bool b_dynrng; 
int i_flags; 
bool b_dontwarn;
int i_nb_channels; 
uint8_t pi_chan_table[AOUT_CHAN_MAX]; 
bool b_synced;
} decoder_sys_t;
#define DYNRNG_TEXT N_("A/52 dynamic range compression")
#define DYNRNG_LONGTEXT N_( "Dynamic range compression makes the loud sounds softer, and the soft " "sounds louder, so you can more easily listen to the stream in a noisy " "environment without disturbing anyone. If you disable the dynamic range ""compression the playback will be more adapted to a movie theater or a " "listening room.")
vlc_module_begin ()
set_shortname( "A/52" )
set_description( N_("ATSC A/52 (AC-3) audio decoder") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACODEC )
add_bool( "a52-dynrng", true, DYNRNG_TEXT, DYNRNG_LONGTEXT, false )
set_capability( "audio decoder", 60 )
set_callbacks( Open, Close )
vlc_module_end ()
static void Interleave( sample_t *restrict p_out, const sample_t *restrict p_in,
unsigned i_nb_channels, uint8_t *restrict pi_chan_table )
{
for( unsigned j = 0; j < i_nb_channels; j++ )
{
for( unsigned i = 0; i < 256; i++ )
{
#if defined(LIBA52_FIXED)
union { uint32_t u; int32_t i; } spl;
spl.u = ((uint32_t)p_in[j * 256 + i]) << 4;
p_out[i * i_nb_channels + pi_chan_table[j]] = spl.i;
#else
p_out[i * i_nb_channels + pi_chan_table[j]] = p_in[j * 256 + i];
#endif
}
}
}
static void Duplicate( sample_t *restrict p_out, const sample_t *restrict p_in )
{
for( unsigned i = 256; i--; )
{
#if defined(LIBA52_FIXED)
union { uint32_t u; int32_t i; } spl;
spl.u = ((uint32_t)*(p_in++)) << 4;
*p_out++ = spl.i;
*p_out++ = spl.i;
#else
sample_t s = *(p_in++);
*p_out++ = s;
*p_out++ = s;
#endif
}
}
static int Decode( decoder_t *p_dec, block_t *p_in_buf )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if (p_in_buf == NULL) 
return VLCDEC_SUCCESS;
#if defined(LIBA52_FIXED)
sample_t i_sample_level = (1 << 24);
#else
sample_t i_sample_level = 1;
#endif
int i_flags = p_sys->i_flags;
size_t i_bytes_per_block = 256 * p_sys->i_nb_channels * sizeof(sample_t);
block_t *p_out_buf = block_Alloc( 6 * i_bytes_per_block );
if( unlikely(p_out_buf == NULL) )
{
block_Release( p_in_buf );
return VLCDEC_SUCCESS;
}
a52_frame( p_sys->p_liba52, p_in_buf->p_buffer,
&i_flags, &i_sample_level, 0 );
if ( (i_flags & A52_CHANNEL_MASK) != (p_sys->i_flags & A52_CHANNEL_MASK)
&& !p_sys->b_dontwarn )
{
msg_Warn( p_dec,
"liba52 couldn't do the requested downmix 0x%x->0x%x",
p_sys->i_flags & A52_CHANNEL_MASK,
i_flags & A52_CHANNEL_MASK );
p_sys->b_dontwarn = 1;
}
if( !p_sys->b_dynrng )
{
a52_dynrng( p_sys->p_liba52, NULL, NULL );
}
for( unsigned i = 0; i < 6; i++ )
{
if( a52_block( p_sys->p_liba52 ) )
msg_Warn( p_dec, "a52_block failed for block %d", i );
sample_t *p_samples = a52_samples( p_sys->p_liba52 );
if ( ((p_sys->i_flags & A52_CHANNEL_MASK) == A52_CHANNEL1
|| (p_sys->i_flags & A52_CHANNEL_MASK) == A52_CHANNEL2
|| (p_sys->i_flags & A52_CHANNEL_MASK) == A52_MONO)
&& (p_dec->fmt_out.audio.i_physical_channels
& (AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT)) )
{
Duplicate( (sample_t *)(p_out_buf->p_buffer + i * i_bytes_per_block),
p_samples );
}
else
{
Interleave( (sample_t *)(p_out_buf->p_buffer + i * i_bytes_per_block),
p_samples, p_sys->i_nb_channels, p_sys->pi_chan_table);
}
}
p_out_buf->i_nb_samples = A52_FRAME_NB; 
p_out_buf->i_dts = p_in_buf->i_dts;
p_out_buf->i_pts = p_in_buf->i_pts;
p_out_buf->i_length = p_in_buf->i_length;
block_Release( p_in_buf );
decoder_QueueAudio( p_dec, p_out_buf );
return VLCDEC_SUCCESS;
}
static int channels_vlc2a52( const audio_format_t *p_audio, int *p_flags )
{
int i_flags = 0;
switch ( p_audio->i_physical_channels & ~AOUT_CHAN_LFE )
{
case AOUT_CHAN_CENTER:
if ( (p_audio->i_physical_channels & AOUT_CHAN_CENTER)
|| (p_audio->i_physical_channels
& (AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT)) )
i_flags = A52_MONO;
else if ( p_audio->i_physical_channels & AOUT_CHAN_LEFT )
i_flags = A52_CHANNEL1;
else
i_flags = A52_CHANNEL2;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT:
if ( p_audio->i_chan_mode & AOUT_CHANMODE_DOLBYSTEREO )
i_flags = A52_DOLBY;
else if ( p_audio->i_chan_mode & AOUT_CHANMODE_DUALMONO )
i_flags = A52_CHANNEL;
else if ( !(p_audio->i_physical_channels & AOUT_CHAN_RIGHT) )
i_flags = A52_CHANNEL1;
else if ( !(p_audio->i_physical_channels & AOUT_CHAN_LEFT) )
i_flags = A52_CHANNEL2;
else
i_flags = A52_STEREO;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT | AOUT_CHAN_CENTER:
i_flags = A52_3F;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT | AOUT_CHAN_REARCENTER:
i_flags = A52_2F1R;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT | AOUT_CHAN_CENTER
| AOUT_CHAN_REARCENTER:
i_flags = A52_3F1R;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT
| AOUT_CHAN_REARLEFT | AOUT_CHAN_REARRIGHT:
i_flags = A52_2F2R;
break;
case AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT | AOUT_CHAN_CENTER
| AOUT_CHAN_REARLEFT | AOUT_CHAN_REARRIGHT:
i_flags = A52_3F2R;
break;
default:
return VLC_EGENERIC;
}
if ( p_audio->i_physical_channels & AOUT_CHAN_LFE )
i_flags |= A52_LFE;
*p_flags = i_flags;
return VLC_SUCCESS;
}
static int Open( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys;
if( p_dec->fmt_in.i_codec != VLC_CODEC_A52
|| p_dec->fmt_in.audio.i_rate == 0
|| p_dec->fmt_in.audio.i_physical_channels == 0
|| p_dec->fmt_in.audio.i_bytes_per_frame == 0
|| p_dec->fmt_in.audio.i_frame_length == 0 )
return VLC_EGENERIC;
p_dec->p_sys = p_sys = malloc( sizeof(decoder_sys_t) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_sys->b_dynrng = var_InheritBool( p_this, "a52-dynrng" );
p_sys->b_dontwarn = 0;
p_sys->i_nb_channels = aout_FormatNbChannels( &p_dec->fmt_in.audio );
if( channels_vlc2a52( &p_dec->fmt_in.audio, &p_sys->i_flags )
!= VLC_SUCCESS )
{
msg_Warn( p_this, "unknown sample format!" );
free( p_sys );
return VLC_EGENERIC;
}
p_sys->i_flags |= A52_ADJUST_LEVEL;
p_sys->p_liba52 = a52_init( 0 );
if( p_sys->p_liba52 == NULL )
{
msg_Err( p_this, "unable to initialize liba52" );
free( p_sys );
return VLC_EGENERIC;
}
static const uint32_t pi_channels_in[] = {
AOUT_CHAN_LFE, AOUT_CHAN_LEFT, AOUT_CHAN_CENTER, AOUT_CHAN_RIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARCENTER, AOUT_CHAN_REARRIGHT, 0
};
aout_CheckChannelReorder( pi_channels_in, NULL,
p_dec->fmt_in.audio.i_physical_channels,
p_sys->pi_chan_table );
p_dec->fmt_out.audio = p_dec->fmt_in.audio;
#if defined(LIBA52_FIXED)
p_dec->fmt_out.audio.i_format = VLC_CODEC_S32N;
#else
p_dec->fmt_out.audio.i_format = VLC_CODEC_FL32;
#endif
p_dec->fmt_out.i_codec = p_dec->fmt_out.audio.i_format;
aout_FormatPrepare( &p_dec->fmt_out.audio );
if( decoder_UpdateAudioFormat( p_dec ) )
{
Close( p_this );
return VLC_EGENERIC;
}
p_dec->pf_decode = Decode;
p_dec->pf_flush = NULL;
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
a52_free( p_sys->p_liba52 );
free( p_sys );
}
