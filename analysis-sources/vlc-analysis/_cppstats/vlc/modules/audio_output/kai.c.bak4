
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>

#include <float.h>

#include <kai.h>

#define FRAME_SIZE 2048

#define AUDIO_BUFFER_SIZE_IN_SECONDS SEC_FROM_VLC_TICK( AOUT_MAX_ADVANCE_TIME )

struct audio_buffer_t
{
uint8_t *data;
int read_pos;
int write_pos;
int length;
int size;
vlc_mutex_t mutex;
vlc_cond_t cond;
};

typedef struct audio_buffer_t audio_buffer_t;







typedef struct
{
audio_buffer_t *buffer;
HKAI hkai;
float soft_gain;
bool soft_mute;
audio_sample_format_t format;
} aout_sys_t;




static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static void Play ( audio_output_t *_p_aout, block_t *block, vlc_tick_t );
static void Pause ( audio_output_t *, bool, vlc_tick_t );
static void Flush ( audio_output_t * );
static int TimeGet ( audio_output_t *, vlc_tick_t *restrict );

static ULONG APIENTRY KaiCallback ( PVOID, PVOID, ULONG );

static int CreateBuffer ( audio_output_t *, int );
static void DestroyBuffer( audio_output_t * );
static int ReadBuffer ( audio_output_t *, uint8_t *, int );
static int WriteBuffer ( audio_output_t *, uint8_t *, int );

#include "audio_output/volume.h"




#define KAI_AUDIO_DEVICE_TEXT N_( "Device" )

#define KAI_AUDIO_DEVICE_LONGTEXT N_( "Select a proper audio device to be used by KAI." )


#define KAI_AUDIO_EXCLUSIVE_MODE_TEXT N_( "Open audio in exclusive mode." )

#define KAI_AUDIO_EXCLUSIVE_MODE_LONGTEXT N_( "Enable this option if you want your audio not to be interrupted by the " "other audio." )



static const char *const ppsz_kai_audio_device[] = {
"auto", "dart", "uniaud" };
static const char *const ppsz_kai_audio_device_text[] = {
N_("Auto"), "DART", "UNIAUD" };

vlc_module_begin ()
set_shortname( "KAI" )
set_description( N_("K Audio Interface audio output") )
set_capability( "audio output", 100 )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AOUT )
add_string( "kai-audio-device", ppsz_kai_audio_device[0],
KAI_AUDIO_DEVICE_TEXT, KAI_AUDIO_DEVICE_LONGTEXT, false )
change_string_list( ppsz_kai_audio_device, ppsz_kai_audio_device_text )
add_sw_gain( )
add_bool( "kai-audio-exclusive-mode", false,
KAI_AUDIO_EXCLUSIVE_MODE_TEXT, KAI_AUDIO_EXCLUSIVE_MODE_LONGTEXT,
true )
set_callbacks( Open, Close )
vlc_module_end ()




static int Start ( audio_output_t *p_aout, audio_sample_format_t *fmt )
{
aout_sys_t *p_sys = p_aout->sys;
char *psz_mode;
ULONG i_kai_mode;
KAISPEC ks_wanted, ks_obtained;
int i_nb_channels;
int i_bytes_per_frame;
vlc_value_t val, text;
audio_sample_format_t format = *fmt;

if( aout_FormatNbChannels( fmt ) == 0 )
return VLC_EGENERIC;

psz_mode = var_InheritString( p_aout, "kai-audio-device" );
if( !psz_mode )
psz_mode = ( char * )ppsz_kai_audio_device[ 0 ]; 

i_kai_mode = KAIM_AUTO;
if( strcmp( psz_mode, "dart" ) == 0 )
i_kai_mode = KAIM_DART;
else if( strcmp( psz_mode, "uniaud" ) == 0 )
i_kai_mode = KAIM_UNIAUD;
msg_Dbg( p_aout, "selected mode = %s", psz_mode );

if( psz_mode != ppsz_kai_audio_device[ 0 ])
free( psz_mode );

i_nb_channels = aout_FormatNbChannels( &format );
if ( i_nb_channels >= 2 )
{

i_nb_channels = 2;
format.i_physical_channels = AOUT_CHANS_STEREO;
}
else
format.i_physical_channels = AOUT_CHAN_CENTER;


format.i_format = VLC_CODEC_S16N;

aout_FormatPrepare( &format );

i_bytes_per_frame = format.i_bytes_per_frame;


if( kaiInit( i_kai_mode ))
{
msg_Err( p_aout, "cannot initialize KAI");

return VLC_EGENERIC;
}

ks_wanted.usDeviceIndex = 0;
ks_wanted.ulType = KAIT_PLAY;
ks_wanted.ulBitsPerSample = BPS_16;
ks_wanted.ulSamplingRate = format.i_rate;
ks_wanted.ulDataFormat = MCI_WAVE_FORMAT_PCM;
ks_wanted.ulChannels = i_nb_channels;
ks_wanted.ulNumBuffers = 2;
ks_wanted.ulBufferSize = FRAME_SIZE * i_bytes_per_frame;
ks_wanted.fShareable = !var_InheritBool( p_aout,
"kai-audio-exclusive-mode");
ks_wanted.pfnCallBack = KaiCallback;
ks_wanted.pCallBackData = p_aout;
msg_Dbg( p_aout, "requested ulBufferSize = %ld", ks_wanted.ulBufferSize );


if( kaiOpen( &ks_wanted, &ks_obtained, &p_sys->hkai ))
{
msg_Err( p_aout, "cannot open KAI device");

goto exit_kai_done;
}

msg_Dbg( p_aout, "open in %s mode",
ks_obtained.fShareable ? "shareable" : "exclusive" );
msg_Dbg( p_aout, "obtained i_nb_samples = %lu",
ks_obtained.ulBufferSize / i_bytes_per_frame );
msg_Dbg( p_aout, "obtained i_bytes_per_frame = %d",
format.i_bytes_per_frame );

format.channel_type = AUDIO_CHANNEL_TYPE_BITMAP;
p_sys->format = *fmt = format;

p_aout->time_get = TimeGet;
p_aout->play = Play;
p_aout->pause = Pause;
p_aout->flush = Flush;

aout_SoftVolumeStart( p_aout );

CreateBuffer( p_aout, AUDIO_BUFFER_SIZE_IN_SECONDS *
format.i_rate * format.i_bytes_per_frame );


_control87(MCW_EM, MCW_EM);

return VLC_SUCCESS;

exit_kai_done :
kaiDone();

return VLC_EGENERIC;
}




static void Play(audio_output_t *p_aout, block_t *block, vlc_tick_t date)
{
aout_sys_t *p_sys = p_aout->sys;

kaiPlay( p_sys->hkai );

WriteBuffer( p_aout, block->p_buffer, block->i_buffer );

block_Release( block );
(void) date;
}




static void Stop ( audio_output_t *p_aout )
{
aout_sys_t *p_sys = p_aout->sys;

kaiClose( p_sys->hkai );
kaiDone();

DestroyBuffer( p_aout );
}




static ULONG APIENTRY KaiCallback( PVOID p_cb_data,
PVOID p_buffer,
ULONG i_buf_size )
{
audio_output_t *p_aout = (audio_output_t *)p_cb_data;
int i_len;

i_len = ReadBuffer( p_aout, p_buffer, i_buf_size );
if(( ULONG )i_len < i_buf_size )
memset(( uint8_t * )p_buffer + i_len, 0, i_buf_size - i_len );

return i_buf_size;
}

static int Open (vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = calloc( 1, sizeof( aout_sys_t ) );

if( unlikely( sys == NULL ))
return VLC_ENOMEM;

aout->sys = sys;
aout->start = Start;
aout->stop = Stop;
aout_SoftVolumeInit( aout );
return VLC_SUCCESS;
}

static void Close( vlc_object_t *obj )
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = aout->sys;

free(sys);
}

static void Pause( audio_output_t *aout, bool pause, vlc_tick_t date )
{
VLC_UNUSED( date );

aout_sys_t *sys = aout->sys;

if( pause )
kaiPause( sys->hkai );
else
kaiResume( sys->hkai );
}

static void Flush( audio_output_t *aout )
{
aout_sys_t *sys = aout->sys;
audio_buffer_t *buffer = sys->buffer;

vlc_mutex_lock( &buffer->mutex );

buffer->read_pos = buffer->write_pos;
buffer->length = 0;

vlc_mutex_unlock( &buffer->mutex );
}

static int TimeGet( audio_output_t *aout, vlc_tick_t *restrict delay )
{
aout_sys_t *sys = aout->sys;
audio_sample_format_t *format = &sys->format;
audio_buffer_t *buffer = sys->buffer;

vlc_mutex_lock( &buffer->mutex );

*delay = vlc_tick_from_samples( buffer->length / format->i_bytes_per_frame,
format->i_rate );

vlc_mutex_unlock( &buffer->mutex );

return 0;
}

static int CreateBuffer( audio_output_t *aout, int size )
{
audio_buffer_t *buffer;

buffer = calloc( 1, sizeof( *buffer ));
if( !buffer )
return -1;

buffer->data = malloc( size );
if( !buffer->data )
{
free( buffer );

return -1;
}

buffer->size = size;

vlc_mutex_init( &buffer->mutex );
vlc_cond_init( &buffer->cond );

aout_sys_t *sys = aout->sys;
sys->buffer = buffer;

return 0;
}

static void DestroyBuffer( audio_output_t *aout )
{
aout_sys_t *sys = aout->sys;
audio_buffer_t *buffer = sys->buffer;

free( buffer->data );
free( buffer );
}

static int ReadBuffer( audio_output_t *aout, uint8_t *data, int size )
{
aout_sys_t *sys = aout->sys;
audio_buffer_t *buffer = sys->buffer;
int len;
int remain_len = 0;

vlc_mutex_lock( &buffer->mutex );

len = MIN( buffer->length, size );
if( buffer->read_pos + len > buffer->size )
{
remain_len = len;
len = buffer->size - buffer->read_pos;
remain_len -= len;
}

memcpy( data, buffer->data + buffer->read_pos, len );
if( remain_len )
memcpy( data + len, buffer->data, remain_len );

len += remain_len;

buffer->read_pos += len;
buffer->read_pos %= buffer->size;

buffer->length -= len;

vlc_cond_signal( &buffer->cond );

vlc_mutex_unlock( &buffer->mutex );

return len;
}

static int WriteBuffer( audio_output_t *aout, uint8_t *data, int size )
{
aout_sys_t *sys = aout->sys;
audio_buffer_t *buffer = sys->buffer;
int len;
int remain_len = 0;

vlc_mutex_lock( &buffer->mutex );




while( buffer->length + size > buffer->size )
vlc_cond_wait( &buffer->cond, &buffer->mutex );

len = size;
if( buffer->write_pos + len > buffer->size )
{
remain_len = len;
len = buffer->size - buffer->write_pos;
remain_len -= len;
}

memcpy( buffer->data + buffer->write_pos, data, len );
if( remain_len )
memcpy( buffer->data, data + len, remain_len );

buffer->write_pos += size;
buffer->write_pos %= buffer->size;

buffer->length += size;

vlc_mutex_unlock( &buffer->mutex );

return size;
}
