#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_meta.h>
#include <vlc_charset.h>
#include <assert.h>
#include <libmodplug/modplug.h>
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
#define NOISE_LONGTEXT N_("Enable noise reduction algorithm.")
#define REVERB_LONGTEXT N_("Enable reverberation" )
#define REVERB_LEVEL_LONGTEXT N_( "Reverberation level (from 0 " "to 100, default value is 0)." )
#define REVERB_DELAY_LONGTEXT N_("Reverberation delay, in ms." " Usual values are from 40 to 200ms." )
#define MEGABASS_LONGTEXT N_( "Enable megabass mode" )
#define MEGABASS_LEVEL_LONGTEXT N_("Megabass mode level (from 0 to 100, " "default value is 0)." )
#define MEGABASS_RANGE_LONGTEXT N_("Megabass mode cutoff frequency, in Hz. " "This is the maximum frequency for which the megabass " "effect applies. Valid values are from 10 to 100 Hz." )
#define SURROUND_LEVEL_LONGTEXT N_( "Surround effect level (from 0 to 100, " "default value is 0)." )
#define SURROUND_DELAY_LONGTEXT N_("Surround delay, in ms. Usual values are " "from 5 to 40 ms." )
vlc_module_begin ()
set_shortname( "MOD")
set_description( N_("MOD demuxer (libmodplug)" ) )
set_capability( "demux", 10 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_DEMUX )
add_bool( "mod-noisereduction", true, N_("Noise reduction"),
NOISE_LONGTEXT, false )
add_bool( "mod-reverb", false, N_("Reverb"),
REVERB_LONGTEXT, false )
add_integer_with_range( "mod-reverb-level", 0, 0, 100,
N_("Reverberation level"), REVERB_LEVEL_LONGTEXT, true )
add_integer_with_range( "mod-reverb-delay", 40, 0, 1000,
N_("Reverberation delay"), REVERB_DELAY_LONGTEXT, true )
add_bool( "mod-megabass", false, N_("Mega bass"),
MEGABASS_LONGTEXT, false )
add_integer_with_range( "mod-megabass-level", 0, 0, 100,
N_("Mega bass level"), MEGABASS_LEVEL_LONGTEXT, true )
add_integer_with_range( "mod-megabass-range", 10, 10, 100,
N_("Mega bass cutoff"), MEGABASS_RANGE_LONGTEXT, true )
add_bool( "mod-surround", false, N_("Surround"), N_("Surround"),
false )
add_integer_with_range( "mod-surround-level", 0, 0, 100,
N_("Surround level"), SURROUND_LEVEL_LONGTEXT, true )
add_integer_with_range( "mod-surround-delay", 5, 0, 1000,
N_("Surround delay (ms)"), SURROUND_DELAY_LONGTEXT, true )
set_callbacks( Open, Close )
add_shortcut( "mod" )
vlc_module_end ()
static vlc_mutex_t libmodplug_lock = VLC_STATIC_MUTEX;
typedef struct
{
es_format_t fmt;
es_out_id_t *es;
date_t pts;
vlc_tick_t i_length;
int i_data;
uint8_t *p_data;
ModPlugFile *f;
} demux_sys_t;
static int Demux ( demux_t *p_demux );
static int Control( demux_t *p_demux, int i_query, va_list args );
static int Validate( demux_t *p_demux, const char *psz_ext );
#define MOD_MAX_FILE_SIZE (500*1000*1000)
static int Open( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t*)p_this;
demux_sys_t *p_sys;
ModPlug_Settings settings;
if( !p_demux->obj.force )
{
const char *psz_ext = p_demux->psz_filepath ? strrchr( p_demux->psz_filepath, '.' )
: NULL;
if( psz_ext )
psz_ext++;
if( Validate( p_demux, psz_ext ) )
{
msg_Dbg( p_demux, "MOD validation failed (ext=%s)", psz_ext ? psz_ext : "");
return VLC_EGENERIC;
}
}
const int64_t i_size = stream_Size( p_demux->s );
if( i_size <= 0 || i_size >= MOD_MAX_FILE_SIZE )
return VLC_EGENERIC;
p_sys = vlc_obj_malloc( p_this, sizeof (*p_sys) );
if( !p_sys )
return VLC_ENOMEM;
msg_Dbg( p_demux, "loading complete file (could be long)" );
p_sys->i_data = i_size;
p_sys->p_data = vlc_obj_malloc( p_this, p_sys->i_data );
if( unlikely(p_sys->p_data == NULL) )
return VLC_ENOMEM;
p_sys->i_data = vlc_stream_Read( p_demux->s, p_sys->p_data,
p_sys->i_data );
if( p_sys->i_data <= 0 )
{
msg_Err( p_demux, "failed to read the complete file" );
return VLC_EGENERIC;
}
vlc_mutex_lock( &libmodplug_lock );
ModPlug_GetSettings( &settings );
settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING;
settings.mChannels = 2;
settings.mBits = 16;
settings.mFrequency = 44100;
settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
if( var_InheritBool( p_demux, "mod-noisereduction" ) )
settings.mFlags |= MODPLUG_ENABLE_NOISE_REDUCTION;
if( var_InheritBool( p_demux, "mod-reverb" ) )
settings.mFlags |= MODPLUG_ENABLE_REVERB;
settings.mReverbDepth = var_InheritInteger( p_demux, "mod-reverb-level" );
settings.mReverbDelay = var_InheritInteger( p_demux, "mod-reverb-delay" );
if( var_InheritBool( p_demux, "mod-megabass" ) )
settings.mFlags |= MODPLUG_ENABLE_MEGABASS;
settings.mBassAmount = var_InheritInteger( p_demux, "mod-megabass-level" );
settings.mBassRange = var_InheritInteger( p_demux, "mod-megabass-range" );
if( var_InheritBool( p_demux, "mod-surround" ) )
settings.mFlags |= MODPLUG_ENABLE_SURROUND;
settings.mSurroundDepth = var_InheritInteger( p_demux, "mod-surround-level" );
settings.mSurroundDelay = var_InheritInteger( p_demux, "mod-surround-delay" );
ModPlug_SetSettings( &settings );
p_sys->f = ModPlug_Load( p_sys->p_data, p_sys->i_data );
vlc_mutex_unlock( &libmodplug_lock );
if( !p_sys->f )
{
msg_Err( p_demux, "failed to understand the file" );
return VLC_EGENERIC;
}
date_Init( &p_sys->pts, settings.mFrequency, 1 );
date_Set( &p_sys->pts, VLC_TICK_0 );
p_sys->i_length = VLC_TICK_FROM_MS( ModPlug_GetLength( p_sys->f ) );
msg_Dbg( p_demux, "MOD loaded name=%s length=%"PRId64"ms",
ModPlug_GetName( p_sys->f ),
MS_FROM_VLC_TICK( p_sys->i_length ) );
#if defined(WORDS_BIGENDIAN)
es_format_Init( &p_sys->fmt, AUDIO_ES, VLC_FOURCC( 't', 'w', 'o', 's' ) );
#else
es_format_Init( &p_sys->fmt, AUDIO_ES, VLC_FOURCC( 'a', 'r', 'a', 'w' ) );
#endif
p_sys->fmt.audio.i_rate = settings.mFrequency;
p_sys->fmt.audio.i_channels = settings.mChannels;
p_sys->fmt.audio.i_bitspersample = settings.mBits;
p_sys->es = es_out_Add( p_demux->out, &p_sys->fmt );
if( unlikely(p_sys->es == NULL) )
return VLC_ENOMEM;
p_demux->pf_demux = Demux;
p_demux->pf_control = Control;
p_demux->p_sys = p_sys;
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t*)p_this;
demux_sys_t *p_sys = p_demux->p_sys;
ModPlug_Unload( p_sys->f );
}
static int Demux( demux_t *p_demux )
{
demux_sys_t *p_sys = p_demux->p_sys;
block_t *p_frame;
const int i_bk = ( p_sys->fmt.audio.i_bitspersample / 8 ) *
p_sys->fmt.audio.i_channels;
p_frame = block_Alloc( p_sys->fmt.audio.i_rate / 10 * i_bk );
if( !p_frame )
return VLC_DEMUXER_EGENERIC;
const int i_read = ModPlug_Read( p_sys->f, p_frame->p_buffer, p_frame->i_buffer );
if( i_read <= 0 )
{
block_Release( p_frame );
return VLC_DEMUXER_EOF;
}
p_frame->i_buffer = i_read;
p_frame->i_dts =
p_frame->i_pts = date_Get( &p_sys->pts );
es_out_SetPCR( p_demux->out, p_frame->i_pts );
es_out_Send( p_demux->out, p_sys->es, p_frame );
date_Increment( &p_sys->pts, i_read / i_bk );
return VLC_DEMUXER_SUCCESS;
}
static int Control( demux_t *p_demux, int i_query, va_list args )
{
demux_sys_t *p_sys = p_demux->p_sys;
double f, *pf;
vlc_tick_t i64;
vlc_tick_t *pi64;
switch( i_query )
{
case DEMUX_CAN_SEEK:
*va_arg( args, bool * ) = true;
return VLC_SUCCESS;
case DEMUX_GET_POSITION:
pf = va_arg( args, double* );
if( p_sys->i_length > 0 )
{
double current = date_Get( &p_sys->pts ) - VLC_TICK_0;
double length = p_sys->i_length;
*pf = current / length;
return VLC_SUCCESS;
}
return VLC_EGENERIC;
case DEMUX_SET_POSITION:
f = va_arg( args, double );
i64 = f * p_sys->i_length;
if( i64 >= 0 && i64 <= p_sys->i_length )
{
ModPlug_Seek( p_sys->f, MS_FROM_VLC_TICK(i64) );
date_Set( &p_sys->pts, VLC_TICK_0 + i64 );
return VLC_SUCCESS;
}
return VLC_EGENERIC;
case DEMUX_GET_TIME:
*va_arg( args, vlc_tick_t * ) = date_Get( &p_sys->pts );
return VLC_SUCCESS;
case DEMUX_GET_LENGTH:
pi64 = va_arg( args, vlc_tick_t * );
*pi64 = p_sys->i_length;
return VLC_SUCCESS;
case DEMUX_SET_TIME:
i64 = va_arg( args, vlc_tick_t );
if( likely(i64 >= 0) && i64 <= p_sys->i_length )
{
ModPlug_Seek( p_sys->f, MS_FROM_VLC_TICK( i64 ) );
date_Set( &p_sys->pts, VLC_TICK_0 + i64 );
return VLC_SUCCESS;
}
return VLC_EGENERIC;
case DEMUX_HAS_UNSUPPORTED_META:
{
bool *pb_bool = va_arg( args, bool* );
*pb_bool = false; 
return VLC_SUCCESS;
}
case DEMUX_GET_META:
{
vlc_meta_t *p_meta = va_arg( args, vlc_meta_t * );
unsigned i_num_samples = ModPlug_NumSamples( p_sys->f ),
i_num_instruments = ModPlug_NumInstruments( p_sys->f );
unsigned i_num_patterns = ModPlug_NumPatterns( p_sys->f ),
i_num_channels = ModPlug_NumChannels( p_sys->f );
char psz_temp[2048]; 
char *psz_module_info, *psz_instrument_info;
unsigned i_temp_index = 0;
const char *psz_name = ModPlug_GetName( p_sys->f );
if( psz_name && *psz_name && IsUTF8( psz_name ) )
vlc_meta_SetTitle( p_meta, psz_name );
psz_name = ModPlug_GetMessage( p_sys->f );
if( psz_name && *psz_name && IsUTF8( psz_name ) )
vlc_meta_SetDescription( p_meta, psz_name );
if( asprintf( &psz_instrument_info, ", %i Instruments",
i_num_instruments ) >= 0 )
{
if( asprintf( &psz_module_info,
"%i Channels, %i Patterns\n"
"%i Samples%s\n",
i_num_channels, i_num_patterns, i_num_samples,
( i_num_instruments ? psz_instrument_info : "" ) ) >= 0 )
{
vlc_meta_AddExtra( p_meta, "Module Information",
psz_module_info );
free( psz_module_info );
}
free( psz_instrument_info );
}
if( i_num_instruments )
{
i_temp_index = 0;
for( unsigned i = 0; i < i_num_instruments && i_temp_index < sizeof(psz_temp); i++ )
{
char lBuffer[33];
ModPlug_InstrumentName( p_sys->f, i, lBuffer );
if ( !lBuffer[0] || !IsUTF8( lBuffer ) ) continue;
i_temp_index += snprintf( &psz_temp[i_temp_index], sizeof(psz_temp) - i_temp_index, "%s\n", lBuffer );
}
vlc_meta_AddExtra( p_meta, "Instruments", psz_temp );
}
for( unsigned int i = 0; i < i_num_samples && i_temp_index < sizeof(psz_temp); i++ )
{
char psz_buffer[33];
ModPlug_SampleName( p_sys->f, i, psz_buffer );
if ( !psz_buffer[0] || !IsUTF8( psz_buffer ) ) continue;
i_temp_index += snprintf( &psz_temp[i_temp_index], sizeof(psz_temp) - i_temp_index, "%s\n", psz_buffer );
}
vlc_meta_AddExtra( p_meta, "Samples", psz_temp );
return VLC_SUCCESS;
}
case DEMUX_GET_FPS: 
return VLC_EGENERIC;
case DEMUX_CAN_PAUSE:
case DEMUX_CAN_CONTROL_PACE:
case DEMUX_GET_PTS_DELAY:
case DEMUX_SET_PAUSE_STATE:
return demux_vaControlHelper( p_demux->s, 0, -1, 0, 1, i_query, args );
default:
return VLC_EGENERIC;
}
}
static int Validate( demux_t *p_demux, const char *psz_ext )
{
static const struct
{
int i_offset;
const char *psz_marker;
} p_marker[] = {
{ 0, "ziRCONia" }, 
{ 0, "Extended Module" }, 
{ 44, "SCRM" }, 
{ 0, "IMPM" }, 
{ 0, "GF1PATCH110" }, 
{ 20, "!SCREAM!" }, 
{ 20, "!Scream!" }, 
{ 20, "BMOD2STM" }, 
{ 0, "MMD0" }, 
{ 0, "MMD1" }, 
{ 0, "MMD2" }, 
{ 0, "MMD3" }, 
{ 0, "MTM" }, 
{ 0, "DMDL" }, 
{ 0, "DBM0" }, 
{ 0, "if" }, 
{ 0, "JN" }, 
{ 0, "FAR\xfe" }, 
{ 0, "Extreme" }, 
{ 0, "OKTASONGCMOD" }, 
{ 44, "PTMF" }, 
{ 0, "MAS_UTrack_V00" }, 
{ 0, "DDMF" }, 
{ 8, "DSMFSONG" }, 
{ 0, "\xc1\x83\x2a\x9e" }, 
{ 0, "ASYLUM Music Format V1.0" }, 
{ 0, "AMF" }, 
{ 0, "PSM\xfe" }, 
{ 0, "PSM " }, 
{ 0, "MT20" }, 
{ 1080, "M.K." }, 
{ 1080, "M!K!" },
{ 1080, "M&K!" },
{ 1080, "N.T." },
{ 1080, "CD81" },
{ 1080, "OKTA" },
{ 1080, "16CN" },
{ 1080, "32CN" },
{ 1080, "FLT4" },
{ 1080, "FLT8" },
{ 1080, "6CHN" },
{ 1080, "8CHN" },
{ 1080, "FLT" },
{ 1080, "TDZ" },
{ 1081, "CHN" },
{ 1082, "CH" },
{ -1, NULL }
};
static const char *ppsz_mod_ext[] =
{
"mod", "s3m", "xm", "it", "669", "amf", "ams", "dbm", "dmf", "dsm",
"far", "mdl", "med", "mtm", "okt", "ptm", "stm", "ult", "umx", "mt2",
"psm", "abc", NULL
};
bool has_valid_extension = false;
if( psz_ext )
{
for( int i = 0; ppsz_mod_ext[i] != NULL; i++ )
{
has_valid_extension |= !strcasecmp( psz_ext, ppsz_mod_ext[i] );
if( has_valid_extension )
break;
}
}
const uint8_t *p_peek;
const int i_peek = vlc_stream_Peek( p_demux->s, &p_peek, 2048 );
if( i_peek < 4 )
return VLC_EGENERIC;
for( int i = 0; p_marker[i].i_offset >= 0; i++ )
{
const char *psz_marker = p_marker[i].psz_marker;
const int i_size = strlen( psz_marker );
const int i_offset = p_marker[i].i_offset;
if( i_peek < i_offset + i_size )
continue;
if( !memcmp( &p_peek[i_offset], psz_marker, i_size ) )
{
if( i_size >= 4 || has_valid_extension )
return VLC_SUCCESS;
}
}
if( psz_ext && !strcasecmp( psz_ext, "abc" ) )
{
bool b_k = false;
bool b_tx = false;
for( int i = 0; i < i_peek-1; i++ )
{
b_k |= p_peek[i+0] == 'K' && p_peek[i+1] == ':';
b_tx |= ( p_peek[i+0] == 'X' || p_peek[i+0] == 'T') && p_peek[i+1] == ':';
}
if( !b_k || !b_tx )
return VLC_EGENERIC;
return VLC_SUCCESS;
}
if( psz_ext && !strcasecmp( psz_ext, "mod" ) && i_peek >= 20 + 15 * 30 )
{
const uint8_t *p = memchr( p_peek, '\0', 20 );
if( p )
{
for( ; p < &p_peek[20]; p++ )
{
if( *p )
return VLC_EGENERIC;
}
}
for( int i = 0; i < 15; i++ )
{
const uint8_t *p_sample = &p_peek[20 + i*30];
p = memchr( &p_sample[0], '\0', 22 );
if( p )
{
for( ; p < &p_sample[22]; p++ )
{
if( *p )
return VLC_EGENERIC;
}
}
if( p_sample[25] > 64 ) 
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
return VLC_EGENERIC;
}
