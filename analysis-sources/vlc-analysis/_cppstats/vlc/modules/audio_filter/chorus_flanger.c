#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
typedef struct filter_sys_t filter_sys_t;
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static block_t *DoWork( filter_t *, block_t * );
static int paramCallback( vlc_object_t *, char const *, vlc_value_t ,
vlc_value_t , void * );
static int reallocate_buffer( filter_t *, filter_sys_t * );
struct filter_sys_t
{
int i_cumulative;
int i_channels, i_sampleRate;
float f_delayTime, f_feedbackGain; 
float f_wetLevel, f_dryLevel;
float f_sweepDepth, f_sweepRate;
float f_offset;
int i_step;
float f_temp;
float f_sinMultiplier;
int i_bufferLength;
float * p_delayLineStart, * p_delayLineEnd;
float * p_write;
};
vlc_module_begin ()
set_description( N_("Sound Delay") )
set_shortname( N_("Delay") )
set_help( N_("Add a delay effect to the sound") )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AFILTER )
add_shortcut( "delay" )
add_float( "delay-time", 20, N_("Delay time"),
N_("Time in milliseconds of the average delay. Note average"), true )
add_float( "sweep-depth", 6, N_("Sweep Depth"),
N_("Time in milliseconds of the maximum sweep depth. Thus, the sweep "
"range will be delay-time +/- sweep-depth."), true )
add_float( "sweep-rate", 6, N_("Sweep Rate"),
N_("Rate of change of sweep depth in milliseconds shift per second "
"of play"), true )
add_float_with_range( "feedback-gain", 0.5, -0.9, 0.9,
N_("Feedback gain"), N_("Gain on Feedback loop"), true )
add_float_with_range( "wet-mix", 0.4, -0.999, 0.999,
N_("Wet mix"), N_("Level of delayed signal"), true )
add_float_with_range( "dry-mix", 0.4, -0.999, 0.999,
N_("Dry Mix"), N_("Level of input signal"), true )
set_capability( "audio filter", 0 )
set_callbacks( Open, Close )
vlc_module_end ()
static inline float small_value(void)
{
return 1.f / 16777216.f;
}
static int Open( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys = malloc( sizeof( *p_sys ) );
if( !p_sys )
return VLC_ENOMEM;
p_sys->i_channels = aout_FormatNbChannels( &p_filter->fmt_in.audio );
p_sys->f_delayTime = var_CreateGetFloat( p_this, "delay-time" );
p_sys->f_sweepDepth = var_CreateGetFloat( p_this, "sweep-depth" );
p_sys->f_sweepRate = var_CreateGetFloat( p_this, "sweep-rate" );
p_sys->f_feedbackGain = var_CreateGetFloat( p_this, "feedback-gain" );
p_sys->f_dryLevel = var_CreateGetFloat( p_this, "dry-mix" );
p_sys->f_wetLevel = var_CreateGetFloat( p_this, "wet-mix" );
var_AddCallback( p_this, "delay-time", paramCallback, p_sys );
var_AddCallback( p_this, "sweep-depth", paramCallback, p_sys );
var_AddCallback( p_this, "sweep-rate", paramCallback, p_sys );
var_AddCallback( p_this, "feedback-gain", paramCallback, p_sys );
var_AddCallback( p_this, "dry-mix", paramCallback, p_sys );
var_AddCallback( p_this, "wet-mix", paramCallback, p_sys );
if( p_sys->f_delayTime < 0.f )
{
msg_Err( p_filter, "Delay Time is invalid" );
free(p_sys);
return VLC_EGENERIC;
}
if( p_sys->f_sweepDepth > p_sys->f_delayTime || p_sys->f_sweepDepth < 0.f )
{
msg_Err( p_filter, "Sweep Depth is invalid" );
free( p_sys );
return VLC_EGENERIC;
}
if( p_sys->f_sweepRate < 0.f )
{
msg_Err( p_filter, "Sweep Rate is invalid" );
free( p_sys );
return VLC_EGENERIC;
}
p_sys->i_bufferLength = p_sys->i_channels * ( (int)( ( p_sys->f_delayTime
+ p_sys->f_sweepDepth ) * p_filter->fmt_in.audio.i_rate/1000 ) + 1 );
msg_Dbg( p_filter , "Buffer length:%d, Channels:%d, Sweep Depth:%f, Delay "
"time:%f, Sweep Rate:%f, Sample Rate: %d", p_sys->i_bufferLength,
p_sys->i_channels, (double) p_sys->f_sweepDepth,
(double) p_sys->f_delayTime, (double) p_sys->f_sweepRate,
p_filter->fmt_in.audio.i_rate );
if( p_sys->i_bufferLength <= 0 )
{
msg_Err( p_filter, "Delay-time, Sample rate or Channels was incorrect" );
free(p_sys);
return VLC_EGENERIC;
}
p_sys->p_delayLineStart = calloc( p_sys->i_bufferLength, sizeof( float ) );
if( !p_sys->p_delayLineStart )
{
free( p_sys );
return VLC_ENOMEM;
}
p_sys->i_cumulative = 0;
p_sys->i_step = p_sys->f_sweepRate > 0 ? 1 : 0;
p_sys->f_offset = 0;
p_sys->f_temp = 0;
p_sys->p_delayLineEnd = p_sys->p_delayLineStart + p_sys->i_bufferLength;
p_sys->p_write = p_sys->p_delayLineStart;
if( p_sys->f_sweepDepth < small_value() ||
p_filter->fmt_in.audio.i_rate < small_value() ) {
p_sys->f_sinMultiplier = 0.f;
}
else {
p_sys->f_sinMultiplier = 11 * p_sys->f_sweepRate /
( 7 * p_sys->f_sweepDepth * p_filter->fmt_in.audio.i_rate ) ;
}
p_sys->i_sampleRate = p_filter->fmt_in.audio.i_rate;
p_filter->fmt_in.audio.i_format = VLC_CODEC_FL32;
aout_FormatPrepare(&p_filter->fmt_in.audio);
p_filter->fmt_out.audio = p_filter->fmt_in.audio;
p_filter->pf_audio_filter = DoWork;
return VLC_SUCCESS;
}
static inline void sanitize( float * f_value )
{
if ( fabsf( *f_value ) < small_value() )
*f_value = 0.f;
}
static block_t *DoWork( filter_t *p_filter, block_t *p_in_buf )
{
struct filter_sys_t *p_sys = p_filter->p_sys;
int i_chan;
unsigned i_samples = p_in_buf->i_nb_samples; 
int i_maxOffset = floorf( p_sys->f_sweepDepth * p_sys->i_sampleRate / 1000 );
float *p_out = (float*)p_in_buf->p_buffer;
float *p_in = (float*)p_in_buf->p_buffer;
float *p_ptr, f_temp = 0;
for( unsigned i = 0; i < i_samples ; i++ )
{
p_sys->i_cumulative += p_sys->i_step;
p_sys->f_offset = sinf( (p_sys->i_cumulative) * p_sys->f_sinMultiplier )
* floorf(p_sys->f_sweepDepth * p_sys->i_sampleRate / 1000);
if( abs( p_sys->i_step ) > 0 )
{
if( p_sys->i_cumulative >= floorf( p_sys->f_sweepDepth *
p_sys->i_sampleRate / p_sys->f_sweepRate ))
{
p_sys->f_offset = i_maxOffset;
p_sys->i_step = -1 * ( p_sys->i_step );
}
if( p_sys->i_cumulative <= floorf( -1 * p_sys->f_sweepDepth *
p_sys->i_sampleRate / p_sys->f_sweepRate ) )
{
p_sys->f_offset = -i_maxOffset;
p_sys->i_step = -1 * ( p_sys->i_step );
}
}
int offset = floorf( p_sys->f_offset );
p_ptr = p_sys->p_write + ( i_maxOffset - offset ) * p_sys->i_channels;
if( p_ptr < p_sys->p_delayLineStart )
{
p_ptr += p_sys->i_bufferLength - p_sys->i_channels;
}
if( p_ptr > p_sys->p_delayLineEnd - 2*p_sys->i_channels )
{
p_ptr -= p_sys->i_bufferLength - p_sys->i_channels;
}
for( i_chan = 0; i_chan < p_sys->i_channels; i_chan++ )
{
f_temp = ( *( p_ptr + i_chan ) );
sanitize(&f_temp);
p_out[i_chan] = p_sys->f_dryLevel * p_in[i_chan] +
p_sys->f_wetLevel * f_temp;
*( p_sys->p_write + i_chan ) = p_in[i_chan] +
p_sys->f_feedbackGain * f_temp;
}
if( p_sys->p_write == p_sys->p_delayLineStart )
for( i_chan = 0; i_chan < p_sys->i_channels; i_chan++ )
*( p_sys->p_delayLineEnd - p_sys->i_channels + i_chan )
= *( p_sys->p_delayLineStart + i_chan );
p_in += p_sys->i_channels;
p_out += p_sys->i_channels;
p_sys->p_write += p_sys->i_channels;
if( p_sys->p_write == p_sys->p_delayLineEnd - p_sys->i_channels )
{
p_sys->p_write = p_sys->p_delayLineStart;
}
}
return p_in_buf;
}
static void Close( vlc_object_t *p_this )
{
filter_t *p_filter = ( filter_t* )p_this;
filter_sys_t *p_sys = p_filter->p_sys;
var_DelCallback( p_this, "delay-time", paramCallback, p_sys );
var_DelCallback( p_this, "sweep-depth", paramCallback, p_sys );
var_DelCallback( p_this, "sweep-rate", paramCallback, p_sys );
var_DelCallback( p_this, "feedback-gain", paramCallback, p_sys );
var_DelCallback( p_this, "wet-mix", paramCallback, p_sys );
var_DelCallback( p_this, "dry-mix", paramCallback, p_sys );
var_Destroy( p_this, "delay-time" );
var_Destroy( p_this, "sweep-depth" );
var_Destroy( p_this, "sweep-rate" );
var_Destroy( p_this, "feedback-gain" );
var_Destroy( p_this, "wet-mix" );
var_Destroy( p_this, "dry-mix" );
free( p_sys->p_delayLineStart );
free( p_sys );
}
static int paramCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = (filter_sys_t *) p_data;
if( !strncmp( psz_var, "delay-time", 10 ) )
{
if( newval.f_float < 0 )
return VLC_SUCCESS;
p_sys->f_delayTime = newval.f_float;
if( !reallocate_buffer( p_filter, p_sys ) )
{
p_sys->f_delayTime = oldval.f_float;
p_sys->i_bufferLength = p_sys->i_channels * ( (int)
( ( p_sys->f_delayTime + p_sys->f_sweepDepth ) * 
p_filter->fmt_in.audio.i_rate/1000 ) + 1 );
}
}
else if( !strncmp( psz_var, "sweep-depth", 11 ) )
{
if( newval.f_float < 0 || newval.f_float > p_sys->f_delayTime)
return VLC_SUCCESS;
p_sys->f_sweepDepth = newval.f_float;
if( !reallocate_buffer( p_filter, p_sys ) )
{
p_sys->f_sweepDepth = oldval.f_float;
p_sys->i_bufferLength = p_sys->i_channels * ( (int)
( ( p_sys->f_delayTime + p_sys->f_sweepDepth ) * 
p_filter->fmt_in.audio.i_rate/1000 ) + 1 );
}
}
else if( !strncmp( psz_var, "sweep-rate", 10 ) )
{
if( newval.f_float > p_sys->f_sweepDepth )
return VLC_SUCCESS;
p_sys->f_sweepRate = newval.f_float;
if( p_sys->f_sweepDepth < small_value() ||
p_filter->fmt_in.audio.i_rate < small_value() ) {
p_sys->f_sinMultiplier = 0.0;
}
else {
p_sys->f_sinMultiplier = 11 * p_sys->f_sweepRate /
( 7 * p_sys->f_sweepDepth * p_filter->fmt_in.audio.i_rate ) ;
}
}
else if( !strncmp( psz_var, "feedback-gain", 13 ) )
p_sys->f_feedbackGain = newval.f_float;
else if( !strncmp( psz_var, "wet-mix", 7 ) )
p_sys->f_wetLevel = newval.f_float;
else if( !strncmp( psz_var, "dry-mix", 7 ) )
p_sys->f_dryLevel = newval.f_float;
return VLC_SUCCESS;
}
static int reallocate_buffer( filter_t *p_filter, filter_sys_t *p_sys )
{
p_sys->i_bufferLength = p_sys->i_channels * ( (int)( ( p_sys->f_delayTime
+ p_sys->f_sweepDepth ) * p_filter->fmt_in.audio.i_rate/1000 ) + 1 );
float *temp = realloc( p_sys->p_delayLineStart, p_sys->i_bufferLength );
if( unlikely( !temp ) )
{
msg_Err( p_filter, "Couldnt reallocate buffer for new delay." );
return 0;
}
p_sys->p_delayLineStart = temp;
p_sys->p_delayLineEnd = p_sys->p_delayLineStart + p_sys->i_bufferLength;
return 1;
}
