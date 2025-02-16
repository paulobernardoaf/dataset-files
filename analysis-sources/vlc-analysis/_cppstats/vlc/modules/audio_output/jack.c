#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <stdio.h>
#include <unistd.h> 
typedef jack_default_audio_sample_t jack_sample_t;
typedef struct
{
jack_ringbuffer_t *p_jack_ringbuffer;
jack_client_t *p_jack_client;
jack_port_t **p_jack_ports;
jack_sample_t **p_jack_buffers;
unsigned int i_channels;
unsigned int i_rate;
jack_nframes_t latency;
float soft_gain;
bool soft_mute;
vlc_tick_t paused; 
} aout_sys_t;
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static void Play ( audio_output_t * p_aout, block_t *, vlc_tick_t );
static void Pause ( audio_output_t *aout, bool paused, vlc_tick_t date );
static void Flush ( audio_output_t *p_aout );
static int TimeGet ( audio_output_t *, vlc_tick_t * );
static int Process ( jack_nframes_t i_frames, void *p_arg );
static int GraphChange ( void *p_arg );
#include "audio_output/volume.h"
#define AUTO_CONNECT_OPTION "jack-auto-connect"
#define AUTO_CONNECT_TEXT N_("Automatically connect to writable clients")
#define AUTO_CONNECT_LONGTEXT N_( "If enabled, this option will automatically connect sound output to the " "first writable JACK clients found." )
#define CONNECT_REGEX_OPTION "jack-connect-regex"
#define CONNECT_REGEX_TEXT N_("Connect to clients matching")
#define CONNECT_REGEX_LONGTEXT N_( "If automatic connection is enabled, only JACK clients whose names " "match this regular expression will be considered for connection." )
#define JACK_NAME_TEXT N_( "JACK client name" )
vlc_module_begin ()
set_shortname( "JACK" )
set_description( N_("JACK audio output") )
set_capability( "audio output", 100 )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AOUT )
add_bool( AUTO_CONNECT_OPTION, true, AUTO_CONNECT_TEXT,
AUTO_CONNECT_LONGTEXT, false )
add_string( CONNECT_REGEX_OPTION, "system", CONNECT_REGEX_TEXT,
CONNECT_REGEX_LONGTEXT, false )
add_string( "jack-name", "", JACK_NAME_TEXT, JACK_NAME_TEXT, false)
add_sw_gain( )
set_callbacks( Open, Close )
vlc_module_end ()
static int Start( audio_output_t *p_aout, audio_sample_format_t *restrict fmt )
{
char *psz_name;
aout_sys_t *p_sys = p_aout->sys;
int status = VLC_SUCCESS;
unsigned int i;
int i_error;
if( aout_FormatNbChannels( fmt ) == 0 )
return VLC_EGENERIC;
p_sys->latency = 0;
p_sys->paused = VLC_TICK_INVALID;
psz_name = var_InheritString( p_aout, "jack-name" );
if( !psz_name || !*psz_name )
{
free( psz_name );
if( asprintf( &psz_name, "vlc_%d", getpid()) == -1 )
return VLC_ENOMEM;
}
p_sys->p_jack_client = jack_client_open( psz_name,
JackNullOption | JackNoStartServer,
NULL );
if( p_sys->p_jack_client == NULL )
{
msg_Err( p_aout, "failed to connect to JACK server" );
status = VLC_EGENERIC;
goto error_out;
}
jack_set_process_callback( p_sys->p_jack_client, Process, p_aout );
jack_set_graph_order_callback ( p_sys->p_jack_client, GraphChange, p_aout );
fmt->i_format = VLC_CODEC_FL32;
p_sys->i_rate = fmt->i_rate = jack_get_sample_rate( p_sys->p_jack_client );
p_aout->play = Play;
p_aout->pause = Pause;
p_aout->flush = Flush;
p_aout->time_get = TimeGet;
aout_SoftVolumeStart( p_aout );
p_sys->i_channels = aout_FormatNbChannels( fmt );
aout_FormatPrepare(fmt);
p_sys->p_jack_ports = malloc( p_sys->i_channels *
sizeof(jack_port_t *) );
if( p_sys->p_jack_ports == NULL )
{
status = VLC_ENOMEM;
goto error_out;
}
p_sys->p_jack_buffers = malloc( p_sys->i_channels *
sizeof(jack_sample_t *) );
if( p_sys->p_jack_buffers == NULL )
{
status = VLC_ENOMEM;
goto error_out;
}
const size_t buf_sz =
samples_from_vlc_tick(AOUT_MAX_ADVANCE_TIME, fmt->i_rate * fmt->i_bytes_per_frame);
p_sys->p_jack_ringbuffer = jack_ringbuffer_create( buf_sz );
if( p_sys->p_jack_ringbuffer == NULL )
{
status = VLC_ENOMEM;
goto error_out;
}
if( jack_ringbuffer_mlock( p_sys->p_jack_ringbuffer ))
{
msg_Warn( p_aout, "failed to lock JACK ringbuffer in memory" );
}
for( i = 0; i < p_sys->i_channels; i++ )
{
char *psz_name_output;
if( asprintf( &psz_name_output, "%s_out_%d", psz_name, i + 1) != -1 )
{
p_sys->p_jack_ports[i] = jack_port_register( p_sys->p_jack_client,
psz_name_output, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
free( psz_name_output );
}
if( p_sys->p_jack_ports[i] == NULL )
{
msg_Err( p_aout, "failed to register a JACK port" );
status = VLC_EGENERIC;
goto error_out;
}
}
i_error = jack_activate( p_sys->p_jack_client );
if( i_error )
{
msg_Err( p_aout, "failed to activate JACK client (error %d)", i_error );
status = VLC_EGENERIC;
goto error_out;
}
if( var_InheritBool( p_aout, AUTO_CONNECT_OPTION ) )
{
unsigned int i_in_ports;
char *psz_regex = var_InheritString( p_aout, CONNECT_REGEX_OPTION );
const char **pp_in_ports = jack_get_ports( p_sys->p_jack_client,
psz_regex, NULL,
JackPortIsInput );
free( psz_regex );
i_in_ports = 0;
while( pp_in_ports && pp_in_ports[i_in_ports] )
{
i_in_ports++;
}
for( i = 0; i_in_ports > 0 && i < p_sys->i_channels; i++ )
{
const char* psz_in = pp_in_ports[i % i_in_ports];
const char* psz_out = jack_port_name( p_sys->p_jack_ports[i] );
i_error = jack_connect( p_sys->p_jack_client, psz_out, psz_in );
if( i_error )
{
msg_Err( p_aout, "failed to connect port %s to port %s (error %d)",
psz_out, psz_in, i_error );
}
else
{
msg_Dbg( p_aout, "connecting port %s to port %s",
psz_out, psz_in );
}
}
free( pp_in_ports );
}
msg_Dbg( p_aout, "JACK audio output initialized (%d channels, rate=%d)",
p_sys->i_channels, fmt->i_rate );
fmt->channel_type = AUDIO_CHANNEL_TYPE_BITMAP;
error_out:
if( status != VLC_SUCCESS )
{
if( p_sys->p_jack_client )
{
jack_deactivate( p_sys->p_jack_client );
jack_client_close( p_sys->p_jack_client );
}
if( p_sys->p_jack_ringbuffer )
jack_ringbuffer_free( p_sys->p_jack_ringbuffer );
free( p_sys->p_jack_ports );
free( p_sys->p_jack_buffers );
}
free( psz_name );
return status;
}
static void Play(audio_output_t * p_aout, block_t * p_block, vlc_tick_t date)
{
aout_sys_t *p_sys = p_aout->sys;
jack_ringbuffer_t *rb = p_sys->p_jack_ringbuffer;
const size_t bytes_per_frame = p_sys->i_channels * sizeof(jack_sample_t);
while (p_block->i_buffer > 0) {
const size_t write_space = jack_ringbuffer_write_space(rb);
const size_t bytes = p_block->i_buffer < write_space ?
p_block->i_buffer : write_space;
if( unlikely( bytes == 0 ) ) {
msg_Warn( p_aout, "%"PRIuPTR " frames of audio dropped",
p_block->i_buffer / bytes_per_frame );
break;
}
jack_ringbuffer_write( rb, (const char *) p_block->p_buffer, bytes );
p_block->p_buffer += bytes;
p_block->i_buffer -= bytes;
}
block_Release(p_block);
(void) date;
}
static void Pause(audio_output_t *aout, bool paused, vlc_tick_t date)
{
aout_sys_t *sys = aout->sys;
if( paused ) {
sys->paused = date;
} else {
date -= sys->paused;
msg_Dbg(aout, "resuming after %"PRId64" us", date);
sys->paused = VLC_TICK_INVALID;
}
}
static void Flush(audio_output_t *p_aout)
{
aout_sys_t * p_sys = p_aout->sys;
jack_ringbuffer_t *rb = p_sys->p_jack_ringbuffer;
jack_ringbuffer_reset(rb);
}
static int TimeGet(audio_output_t *p_aout, vlc_tick_t *delay)
{
aout_sys_t * p_sys = p_aout->sys;
jack_ringbuffer_t *rb = p_sys->p_jack_ringbuffer;
const size_t bytes_per_frame = p_sys->i_channels * sizeof(jack_sample_t);
*delay = p_sys->latency +
vlc_tick_from_samples(jack_ringbuffer_read_space(rb) / bytes_per_frame,
p_sys->i_rate);
return 0;
}
int Process( jack_nframes_t i_frames, void *p_arg )
{
unsigned int i, j, frames_from_rb = 0;
size_t bytes_read = 0;
size_t frames_read;
audio_output_t *p_aout = (audio_output_t*) p_arg;
aout_sys_t *p_sys = p_aout->sys;
if( p_sys->paused == VLC_TICK_INVALID )
frames_from_rb = i_frames;
for( i = 0; i < p_sys->i_channels; i++ )
{
p_sys->p_jack_buffers[i] = jack_port_get_buffer( p_sys->p_jack_ports[i],
i_frames );
}
for( j = 0; j < frames_from_rb; j++ )
{
for( i = 0; i < p_sys->i_channels; i++ )
{
jack_sample_t *p_dst = p_sys->p_jack_buffers[i] + j;
bytes_read += jack_ringbuffer_read( p_sys->p_jack_ringbuffer,
(char *) p_dst, sizeof(jack_sample_t) );
}
}
frames_read = (bytes_read / sizeof(jack_sample_t)) / p_sys->i_channels;
if( frames_read < i_frames )
{
for( i = 0; i < p_sys->i_channels; i++ )
{
memset( p_sys->p_jack_buffers[i] + frames_read, 0,
sizeof( jack_sample_t ) * (i_frames - frames_read) );
}
}
return 0;
}
static int GraphChange( void *p_arg )
{
audio_output_t *p_aout = (audio_output_t*) p_arg;
aout_sys_t *p_sys = p_aout->sys;
unsigned int i;
jack_latency_range_t port_latency;
p_sys->latency = 0;
for( i = 0; i < p_sys->i_channels; ++i )
{
jack_port_get_latency_range( p_sys->p_jack_ports[i], JackPlaybackLatency,
&port_latency );
p_sys->latency = __MAX( p_sys->latency, port_latency.max );
}
msg_Dbg(p_aout, "JACK graph reordered. Our maximum latency=%d.",
p_sys->latency);
return 0;
}
static void Stop( audio_output_t *p_aout )
{
int i_error;
aout_sys_t *p_sys = p_aout->sys;
i_error = jack_deactivate( p_sys->p_jack_client );
if( i_error )
{
msg_Err( p_aout, "jack_deactivate failed (error %d)", i_error );
}
i_error = jack_client_close( p_sys->p_jack_client );
if( i_error )
{
msg_Err( p_aout, "jack_client_close failed (error %d)", i_error );
}
free( p_sys->p_jack_ports );
free( p_sys->p_jack_buffers );
jack_ringbuffer_free( p_sys->p_jack_ringbuffer );
}
static int Open(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = calloc(1, sizeof (*sys));
if( unlikely( sys == NULL ) )
return VLC_ENOMEM;
aout->sys = sys;
aout->start = Start;
aout->stop = Stop;
aout_SoftVolumeInit(aout);
return VLC_SUCCESS;
}
static void Close(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = aout->sys;
free(sys);
}
