#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h>
#include <stdint.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
#define A_TBL (256)
#define DB_TABLE_SIZE (1024)
#define DB_MIN (-60.0f)
#define DB_MAX (24.0f)
#define LIN_TABLE_SIZE (1024)
#define LIN_MIN (0.0000000002f)
#define LIN_MAX (9.0f)
#define DB_DEFAULT_CUBE
#define RMS_BUF_SIZE (960)
#define LOOKAHEAD_SIZE ((RMS_BUF_SIZE)<<1)
#define LIN_INTERP(f,a,b) ((a) + (f) * ( (b) - (a) ))
#define LIMIT(v,l,u) (v < l ? l : ( v > u ? u : v ))
typedef struct
{
float pf_buf[RMS_BUF_SIZE];
unsigned int i_pos;
unsigned int i_count;
float f_sum;
} rms_env;
typedef struct
{
struct
{
float pf_vals[AOUT_CHAN_MAX];
float f_lev_in;
} p_buf[LOOKAHEAD_SIZE];
unsigned int i_pos;
unsigned int i_count;
} lookahead;
typedef struct
{
float f_amp;
float pf_as[A_TBL];
unsigned int i_count;
float f_env;
float f_env_peak;
float f_env_rms;
float f_gain;
float f_gain_out;
rms_env rms;
float f_sum;
lookahead la;
float pf_db_data[DB_TABLE_SIZE];
float pf_lin_data[LIN_TABLE_SIZE];
vlc_mutex_t lock;
float f_rms_peak;
float f_attack;
float f_release;
float f_threshold;
float f_ratio;
float f_knee;
float f_makeup_gain;
} filter_sys_t;
typedef union
{
float f;
int32_t i;
} ls_pcast32;
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static block_t *DoWork ( filter_t *, block_t * );
static void DbInit ( filter_sys_t * );
static float Db2Lin ( float, filter_sys_t * );
static float Lin2Db ( float, filter_sys_t * );
#if defined(DB_DEFAULT_CUBE)
static float CubeInterp ( const float, const float, const float,
const float, const float );
#endif
static void RoundToZero ( float * );
static float Max ( float, float );
static float Clamp ( float, float, float );
static int Round ( float );
static float RmsEnvProcess ( rms_env *, const float );
static void BufferProcess ( float *, int, float, float, lookahead * );
static int RMSPeakCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int AttackCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int ReleaseCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int ThresholdCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int RatioCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int KneeCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
static int MakeupGainCallback ( vlc_object_t *, char const *, vlc_value_t,
vlc_value_t, void * );
#define RMS_PEAK_TEXT N_( "RMS/peak" )
#define RMS_PEAK_LONGTEXT N_( "Set the RMS/peak." )
#define ATTACK_TEXT N_( "Attack time" )
#define ATTACK_LONGTEXT N_( "Set the attack time in milliseconds." )
#define RELEASE_TEXT N_( "Release time" )
#define RELEASE_LONGTEXT N_( "Set the release time in milliseconds." )
#define THRESHOLD_TEXT N_( "Threshold level" )
#define THRESHOLD_LONGTEXT N_( "Set the threshold level in dB." )
#define RATIO_TEXT N_( "Ratio" )
#define RATIO_LONGTEXT N_( "Set the ratio (n:1)." )
#define KNEE_TEXT N_( "Knee radius" )
#define KNEE_LONGTEXT N_( "Set the knee radius in dB." )
#define MAKEUP_GAIN_TEXT N_( "Makeup gain" )
#define MAKEUP_GAIN_LONGTEXT N_( "Set the makeup gain in dB (0 ... 24)." )
vlc_module_begin()
set_shortname( N_("Compressor") )
set_description( N_("Dynamic range compressor") )
set_capability( "audio filter", 0 )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AFILTER )
add_float_with_range( "compressor-rms-peak", 0.2, 0.0, 1.0,
RMS_PEAK_TEXT, RMS_PEAK_LONGTEXT, false )
add_float_with_range( "compressor-attack", 25.0, 1.5, 400.0,
ATTACK_TEXT, ATTACK_LONGTEXT, false )
add_float_with_range( "compressor-release", 100.0, 2.0, 800.0,
RELEASE_TEXT, RELEASE_LONGTEXT, false )
add_float_with_range( "compressor-threshold", -11.0, -30.0, 0.0,
THRESHOLD_TEXT, THRESHOLD_LONGTEXT, false )
add_float_with_range( "compressor-ratio", 4.0, 1.0, 20.0,
RATIO_TEXT, RATIO_LONGTEXT, false )
add_float_with_range( "compressor-knee", 5.0, 1.0, 10.0,
KNEE_TEXT, KNEE_LONGTEXT, false )
add_float_with_range( "compressor-makeup-gain", 7.0, 0.0, 24.0,
MAKEUP_GAIN_TEXT, MAKEUP_GAIN_LONGTEXT, false )
set_callbacks( Open, Close )
add_shortcut( "compressor" )
vlc_module_end ()
static int Open( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
vlc_object_t *p_aout = vlc_object_parent(p_filter);
float f_sample_rate = p_filter->fmt_in.audio.i_rate;
float f_num;
filter_sys_t *p_sys = p_filter->p_sys = calloc( 1, sizeof(*p_sys) );
if( !p_sys )
{
return VLC_ENOMEM;
}
p_sys->pf_as[0] = 1.0f;
for( int i = 1; i < A_TBL; i++ )
{
p_sys->pf_as[i] = expf( -1.0f / ( f_sample_rate * i / A_TBL ) );
}
f_num = 0.01f * f_sample_rate;
p_sys->rms.i_count = Round( Clamp( 0.5f * f_num, 1.0f, RMS_BUF_SIZE ) );
p_sys->la.i_count = Round( Clamp( f_num, 1.0f, LOOKAHEAD_SIZE ) );
DbInit( p_sys );
p_sys->f_rms_peak = var_CreateGetFloat( p_aout, "compressor-rms-peak" );
p_sys->f_attack = var_CreateGetFloat( p_aout, "compressor-attack" );
p_sys->f_release = var_CreateGetFloat( p_aout, "compressor-release" );
p_sys->f_threshold = var_CreateGetFloat( p_aout, "compressor-threshold" );
p_sys->f_ratio = var_CreateGetFloat( p_aout, "compressor-ratio" );
p_sys->f_knee = var_CreateGetFloat( p_aout, "compressor-knee" );
p_sys->f_makeup_gain =
var_CreateGetFloat( p_aout, "compressor-makeup-gain" );
vlc_mutex_init( &p_sys->lock );
var_AddCallback( p_aout, "compressor-rms-peak", RMSPeakCallback, p_sys );
var_AddCallback( p_aout, "compressor-attack", AttackCallback, p_sys );
var_AddCallback( p_aout, "compressor-release", ReleaseCallback, p_sys );
var_AddCallback( p_aout, "compressor-threshold", ThresholdCallback, p_sys );
var_AddCallback( p_aout, "compressor-ratio", RatioCallback, p_sys );
var_AddCallback( p_aout, "compressor-knee", KneeCallback, p_sys );
var_AddCallback( p_aout, "compressor-makeup-gain", MakeupGainCallback, p_sys );
p_filter->fmt_in.audio.i_format = VLC_CODEC_FL32;
aout_FormatPrepare(&p_filter->fmt_in.audio);
p_filter->fmt_out.audio = p_filter->fmt_in.audio;
p_filter->pf_audio_filter = DoWork;
msg_Dbg( p_filter, "compressor successfully initialized" );
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
vlc_object_t *p_aout = vlc_object_parent(p_filter);
filter_sys_t *p_sys = p_filter->p_sys;
var_DelCallback( p_aout, "compressor-rms-peak", RMSPeakCallback, p_sys );
var_DelCallback( p_aout, "compressor-attack", AttackCallback, p_sys );
var_DelCallback( p_aout, "compressor-release", ReleaseCallback, p_sys );
var_DelCallback( p_aout, "compressor-threshold", ThresholdCallback, p_sys );
var_DelCallback( p_aout, "compressor-ratio", RatioCallback, p_sys );
var_DelCallback( p_aout, "compressor-knee", KneeCallback, p_sys );
var_DelCallback( p_aout, "compressor-makeup-gain", MakeupGainCallback, p_sys );
free( p_sys );
}
static block_t * DoWork( filter_t * p_filter, block_t * p_in_buf )
{
int i_samples = p_in_buf->i_nb_samples;
int i_channels = aout_FormatNbChannels( &p_filter->fmt_in.audio );
float *pf_buf = (float*)p_in_buf->p_buffer;
filter_sys_t *p_sys = p_filter->p_sys;
vlc_mutex_lock( &p_sys->lock );
float f_rms_peak = p_sys->f_rms_peak; 
float f_attack = p_sys->f_attack; 
float f_release = p_sys->f_release; 
float f_threshold = p_sys->f_threshold; 
float f_ratio = p_sys->f_ratio; 
float f_knee = p_sys->f_knee; 
float f_makeup_gain = p_sys->f_makeup_gain; 
vlc_mutex_unlock( &p_sys->lock );
float f_amp = p_sys->f_amp;
float *pf_as = p_sys->pf_as;
float f_env = p_sys->f_env;
float f_env_peak = p_sys->f_env_peak;
float f_env_rms = p_sys->f_env_rms;
float f_gain = p_sys->f_gain;
float f_gain_out = p_sys->f_gain_out;
rms_env *p_rms = &p_sys->rms;
float f_sum = p_sys->f_sum;
lookahead *p_la = &p_sys->la;
float f_ga = f_attack < 2.0f ? 0.0f :
pf_as[Round( f_attack * 0.001f * ( A_TBL - 1 ) )];
float f_gr = pf_as[Round( f_release * 0.001f * ( A_TBL - 1 ) )];
float f_rs = ( f_ratio - 1.0f ) / f_ratio;
float f_mug = Db2Lin( f_makeup_gain, p_sys );
float f_knee_min = Db2Lin( f_threshold - f_knee, p_sys );
float f_knee_max = Db2Lin( f_threshold + f_knee, p_sys );
float f_ef_a = f_ga * 0.25f;
float f_ef_ai = 1.0f - f_ef_a;
for( int i = 0; i < i_samples; i++ )
{
float f_lev_in_old, f_lev_in_new;
f_lev_in_old = p_la->p_buf[p_la->i_pos].f_lev_in;
f_lev_in_new = fabs( pf_buf[0] );
for( int i_chan = 1; i_chan < i_channels; i_chan++ )
{
f_lev_in_new = Max( f_lev_in_new, fabs( pf_buf[i_chan] ) );
}
p_la->p_buf[p_la->i_pos].f_lev_in = f_lev_in_new;
f_sum += f_lev_in_new * f_lev_in_new;
if( f_amp > f_env_rms )
{
f_env_rms = f_env_rms * f_ga + f_amp * ( 1.0f - f_ga );
}
else
{
f_env_rms = f_env_rms * f_gr + f_amp * ( 1.0f - f_gr );
}
RoundToZero( &f_env_rms );
if( f_lev_in_old > f_env_peak )
{
f_env_peak = f_env_peak * f_ga + f_lev_in_old * ( 1.0f - f_ga );
}
else
{
f_env_peak = f_env_peak * f_gr + f_lev_in_old * ( 1.0f - f_gr );
}
RoundToZero( &f_env_peak );
if( ( p_sys->i_count++ & 3 ) == 3 )
{
f_amp = RmsEnvProcess( p_rms, f_sum * 0.25f );
f_sum = 0.0f;
if( isnan( f_env_rms ) )
{
f_env_rms = 0.0f;
}
f_env = LIN_INTERP( f_rms_peak, f_env_rms, f_env_peak );
if( f_env <= f_knee_min )
{
f_gain_out = 1.0f;
}
else if( f_env < f_knee_max )
{
const float f_x = -( f_threshold
- f_knee - Lin2Db( f_env, p_sys ) ) / f_knee;
f_gain_out = Db2Lin( -f_knee * f_rs * f_x * f_x * 0.25f,
p_sys );
}
else
{
f_gain_out = Db2Lin( ( f_threshold - Lin2Db( f_env, p_sys ) )
* f_rs, p_sys );
}
}
f_gain = f_gain * f_ef_a + f_gain_out * f_ef_ai;
BufferProcess( pf_buf, i_channels, f_gain, f_mug, p_la );
pf_buf += i_channels;
}
p_sys->f_sum = f_sum;
p_sys->f_amp = f_amp;
p_sys->f_gain = f_gain;
p_sys->f_gain_out = f_gain_out;
p_sys->f_env = f_env;
p_sys->f_env_rms = f_env_rms;
p_sys->f_env_peak = f_env_peak;
return p_in_buf;
}
static void DbInit( filter_sys_t * p_sys )
{
float *pf_lin_data = p_sys->pf_lin_data;
float *pf_db_data = p_sys->pf_db_data;
for( int i = 0; i < LIN_TABLE_SIZE; i++ )
{
pf_lin_data[i] = powf( 10.0f, ( ( DB_MAX - DB_MIN ) *
(float)i / LIN_TABLE_SIZE + DB_MIN ) / 20.0f );
}
for( int i = 0; i < DB_TABLE_SIZE; i++ )
{
pf_db_data[i] = 20.0f * log10f( ( LIN_MAX - LIN_MIN ) *
(float)i / DB_TABLE_SIZE + LIN_MIN );
}
}
static float Db2Lin( float f_db, filter_sys_t * p_sys )
{
float f_scale = ( f_db - DB_MIN ) * LIN_TABLE_SIZE / ( DB_MAX - DB_MIN );
int i_base = Round( f_scale - 0.5f );
float f_ofs = f_scale - i_base;
float *pf_lin_data = p_sys->pf_lin_data;
if( i_base < 1 )
{
return 0.0f;
}
else if( i_base > LIN_TABLE_SIZE - 3 )
{
return pf_lin_data[LIN_TABLE_SIZE - 2];
}
#if defined(DB_DEFAULT_CUBE)
return CubeInterp( f_ofs, pf_lin_data[i_base - 1],
pf_lin_data[i_base],
pf_lin_data[i_base + 1],
pf_lin_data[i_base + 2] );
#else
return ( 1.0f - f_ofs ) * pf_lin_data[i_base]
+ f_ofs * pf_lin_data[i_base + 1];
#endif
}
static float Lin2Db( float f_lin, filter_sys_t * p_sys )
{
float f_scale = ( f_lin - LIN_MIN ) * DB_TABLE_SIZE / ( LIN_MAX - LIN_MIN );
int i_base = Round( f_scale - 0.5f );
float f_ofs = f_scale - i_base;
float *pf_db_data = p_sys->pf_db_data;
if( i_base < 2 )
{
return pf_db_data[2] * f_scale * 0.5f - 23.0f * ( 2.0f - f_scale );
}
else if( i_base > DB_TABLE_SIZE - 3 )
{
return pf_db_data[DB_TABLE_SIZE - 2];
}
#if defined(DB_DEFAULT_CUBE)
return CubeInterp( f_ofs, pf_db_data[i_base - 1],
pf_db_data[i_base],
pf_db_data[i_base + 1],
pf_db_data[i_base + 2] );
#else
return ( 1.0f - f_ofs ) * pf_db_data[i_base]
+ f_ofs * pf_db_data[i_base + 1];
#endif
}
#if defined(DB_DEFAULT_CUBE)
static float CubeInterp( const float f_fr, const float f_inm1,
const float f_in,
const float f_inp1,
const float f_inp2 )
{
return f_in + 0.5f * f_fr * ( f_inp1 - f_inm1 +
f_fr * ( 4.0f * f_inp1 + 2.0f * f_inm1 - 5.0f * f_in - f_inp2 +
f_fr * ( 3.0f * ( f_in - f_inp1 ) - f_inm1 + f_inp2 ) ) );
}
#endif
static void RoundToZero( float *pf_x )
{
static const float f_anti_denormal = 1e-18;
*pf_x += f_anti_denormal;
*pf_x -= f_anti_denormal;
}
static float Max( float f_x, float f_a )
{
f_x -= f_a;
f_x += fabsf( f_x );
f_x *= 0.5f;
f_x += f_a;
return f_x;
}
static float Clamp( float f_x, float f_a, float f_b )
{
const float f_x1 = fabsf( f_x - f_a );
const float f_x2 = fabsf( f_x - f_b );
f_x = f_x1 + f_a + f_b;
f_x -= f_x2;
f_x *= 0.5f;
return f_x;
}
static int Round( float f_x )
{
ls_pcast32 p;
p.f = f_x;
p.f += ( 3 << 22 );
return p.i - 0x4b400000;
}
static float RmsEnvProcess( rms_env * p_r, const float f_x )
{
p_r->f_sum -= p_r->pf_buf[p_r->i_pos];
p_r->f_sum += f_x;
if( p_r->f_sum < 1.0e-6f )
{
p_r->f_sum = 0.0f;
}
p_r->pf_buf[p_r->i_pos] = f_x;
p_r->i_pos = ( p_r->i_pos + 1 ) % ( p_r->i_count );
return sqrt( p_r->f_sum / p_r->i_count );
}
static void BufferProcess( float * pf_buf, int i_channels, float f_gain,
float f_mug, lookahead * p_la )
{
for( int i_chan = 0; i_chan < i_channels; i_chan++ )
{
float f_x = pf_buf[i_chan]; 
pf_buf[i_chan] = p_la->p_buf[p_la->i_pos].pf_vals[i_chan]
* f_gain * f_mug;
p_la->p_buf[p_la->i_pos].pf_vals[i_chan] = f_x;
}
p_la->i_pos = ( p_la->i_pos + 1 ) % ( p_la->i_count );
}
static int RMSPeakCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_rms_peak = Clamp( newval.f_float, 0.0f, 1.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int AttackCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_attack = Clamp( newval.f_float, 1.5f, 400.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int ReleaseCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_release = Clamp( newval.f_float, 2.0f, 800.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int ThresholdCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_threshold = Clamp( newval.f_float, -30.0f, 0.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int RatioCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_ratio = Clamp( newval.f_float, 1.0f, 20.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int KneeCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_knee = Clamp( newval.f_float, 1.0f, 10.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static int MakeupGainCallback( vlc_object_t *p_this, char const *psz_cmd,
vlc_value_t oldval, vlc_value_t newval,
void * p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(psz_cmd); VLC_UNUSED(oldval);
filter_sys_t *p_sys = p_data;
vlc_mutex_lock( &p_sys->lock );
p_sys->f_makeup_gain = Clamp( newval.f_float, 0.0f, 24.0f );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
