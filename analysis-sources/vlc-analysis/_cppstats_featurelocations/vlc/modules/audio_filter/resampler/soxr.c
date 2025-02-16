

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_filter.h>
#include <vlc_plugin.h>

#include <assert.h>
#include <math.h>
#include <soxr.h>

#define SOXR_QUALITY_TEXT N_( "Resampling quality" )
#define SOXR_QUALITY_LONGTEXT N_( "Resampling quality, from worst to best" )

static const int soxr_resampler_quality_vlclist[] = { 0, 1, 2, 3, 4 };
static const char *const soxr_resampler_quality_vlctext[] =
{
"Quick cubic interpolation",
"Low 16-bit with larger roll-off",
"Medium 16-bit with medium roll-off",
"High quality",
"Very high quality"
};
static const soxr_datatype_t soxr_resampler_quality_list[] =
{
SOXR_QQ,
SOXR_LQ,
SOXR_MQ,
SOXR_HQ,
SOXR_VHQ
};
#define MAX_SOXR_QUALITY 4

static int OpenConverter( vlc_object_t * );
static int OpenResampler( vlc_object_t * );
static void Close( vlc_object_t * );

vlc_module_begin ()
set_shortname( N_("SoX Resampler") )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_RESAMPLER )
add_integer( "soxr-resampler-quality", 2,
SOXR_QUALITY_TEXT, NULL, true )
change_integer_list( soxr_resampler_quality_vlclist,
soxr_resampler_quality_vlctext )
set_capability ( "audio converter", 51 )
set_callbacks( OpenConverter, Close )

add_submodule()
set_capability( "audio resampler", 51 )
set_callbacks( OpenResampler, Close )
add_shortcut( "soxr" )
vlc_module_end ()

typedef struct
{
soxr_t soxr;
soxr_t vr_soxr;
soxr_t last_soxr;
double f_fixed_ratio;
size_t i_last_olen;
vlc_tick_t i_last_pts;
} filter_sys_t;

static block_t *Resample( filter_t *, block_t * );
static block_t *Drain( filter_t * );
static void Flush( filter_t * );

static bool
SoXR_GetFormat( vlc_fourcc_t i_format, soxr_datatype_t *p_type )
{
switch( i_format )
{
case VLC_CODEC_FL64:
*p_type = SOXR_FLOAT64_I;
return true;
case VLC_CODEC_FL32:
*p_type = SOXR_FLOAT32_I;
return true;
case VLC_CODEC_S32N:
*p_type = SOXR_INT32_I;
return true;
case VLC_CODEC_S16N:
*p_type = SOXR_INT16_I;
return true;
default:
return false;
}
}

static int
Open( vlc_object_t *p_obj, bool b_change_ratio )
{
filter_t *p_filter = (filter_t *)p_obj;


if( p_filter->fmt_in.audio.i_channels != p_filter->fmt_out.audio.i_channels )
return VLC_EGENERIC;


soxr_datatype_t i_itype, i_otype;
if( !SoXR_GetFormat( p_filter->fmt_in.audio.i_format, &i_itype )
|| !SoXR_GetFormat( p_filter->fmt_out.audio.i_format, &i_otype ) )
return VLC_EGENERIC;

filter_sys_t *p_sys = calloc( 1, sizeof( filter_sys_t ) );
if( unlikely( p_sys == NULL ) )
return VLC_ENOMEM;


int64_t i_vlc_q = var_InheritInteger( p_obj, "soxr-resampler-quality" );
if( i_vlc_q < 0 )
i_vlc_q = 0;
else if( i_vlc_q > MAX_SOXR_QUALITY )
i_vlc_q = MAX_SOXR_QUALITY;
const unsigned long i_recipe = soxr_resampler_quality_list[i_vlc_q];
const unsigned i_channels = p_filter->fmt_in.audio.i_channels;
const double f_ratio = p_filter->fmt_out.audio.i_rate
/ (double) p_filter->fmt_in.audio.i_rate;

p_sys->f_fixed_ratio = f_ratio;
soxr_error_t error;

soxr_io_spec_t io_spec = soxr_io_spec( i_itype, i_otype );

soxr_quality_spec_t q_spec = soxr_quality_spec( i_recipe, 0 );

p_sys->soxr = soxr_create( 1, f_ratio, i_channels,
&error, &io_spec, &q_spec, NULL );
if( error )
{
msg_Err( p_filter, "soxr_create failed: %s", soxr_strerror( error ) );
free( p_sys );
return VLC_EGENERIC;
}




if( b_change_ratio )
{
q_spec = soxr_quality_spec( SOXR_LQ, SOXR_VR );
p_sys->vr_soxr = soxr_create( 1, f_ratio, i_channels,
&error, &io_spec, &q_spec, NULL );
if( error )
{
msg_Err( p_filter, "soxr_create failed: %s", soxr_strerror( error ) );
soxr_delete( p_sys->soxr );
free( p_sys );
return VLC_EGENERIC;
}
soxr_set_io_ratio( p_sys->vr_soxr, 1 / f_ratio, 0 );
}

msg_Dbg( p_filter, "Using SoX Resampler with '%s' engine and '%s' quality "
"to convert %4.4s/%dHz to %4.4s/%dHz.",
soxr_engine( p_sys->soxr ), soxr_resampler_quality_vlctext[i_vlc_q],
(const char *)&p_filter->fmt_in.audio.i_format,
p_filter->fmt_in.audio.i_rate,
(const char *)&p_filter->fmt_out.audio.i_format,
p_filter->fmt_out.audio.i_rate );

p_filter->p_sys = p_sys;
p_filter->pf_audio_filter = Resample;
p_filter->pf_flush = Flush;
p_filter->pf_audio_drain = Drain;
return VLC_SUCCESS;
}

static int
OpenResampler( vlc_object_t *p_obj )
{
filter_t *p_filter = (filter_t *)p_obj;


if( p_filter->fmt_in.audio.i_format != p_filter->fmt_out.audio.i_format )
return VLC_EGENERIC;
return Open( p_obj, true );
}

static int
OpenConverter( vlc_object_t *p_obj )
{
filter_t *p_filter = (filter_t *)p_obj;



if( p_filter->fmt_in.audio.i_rate == p_filter->fmt_out.audio.i_rate )
return VLC_EGENERIC;
return Open( p_obj, false );
}

static void
Close( vlc_object_t *p_obj )
{
filter_t *p_filter = (filter_t *)p_obj;
filter_sys_t *p_sys = p_filter->p_sys;

soxr_delete( p_sys->soxr );
if( p_sys->vr_soxr )
soxr_delete( p_sys->vr_soxr );

free( p_sys );
}

static block_t *
SoXR_Resample( filter_t *p_filter, soxr_t soxr, block_t *p_in, size_t i_olen )
{
filter_sys_t *p_sys = p_filter->p_sys;
size_t i_idone, i_odone;
const size_t i_oframesize = p_filter->fmt_out.audio.i_bytes_per_frame;
const size_t i_ilen = p_in ? p_in->i_nb_samples : 0;

block_t *p_out;
if( i_ilen >= i_olen )
{
i_olen = i_ilen;
p_out = p_in;
}
else
p_out = block_Alloc( i_olen * i_oframesize );

soxr_error_t error = soxr_process( soxr, p_in ? p_in->p_buffer : NULL,
i_ilen, &i_idone, p_out->p_buffer,
i_olen, &i_odone );
if( error )
{
msg_Err( p_filter, "soxr_process failed: %s", soxr_strerror( error ) );
block_Release( p_out );
goto error;
}
if( unlikely( i_idone < i_ilen ) )
msg_Err( p_filter, "lost %zd of %zd input frames",
i_ilen - i_idone, i_idone);

p_out->i_buffer = i_odone * i_oframesize;
p_out->i_nb_samples = i_odone;
p_out->i_length = vlc_tick_from_samples(i_odone, p_filter->fmt_out.audio.i_rate);

if( p_in )
{
p_sys->i_last_olen = i_olen;
p_sys->last_soxr = soxr;
}
else
{
soxr_clear( soxr );
p_sys->i_last_olen = 0;
p_sys->last_soxr = NULL;
}

error:
if( p_in && p_out != p_in )
block_Release( p_in );

return p_out;
}

static size_t
SoXR_GetOutLen( size_t i_ilen, double f_ratio )
{


return lrint( ( i_ilen + 2 ) * f_ratio * 11. / 10. );
}

static block_t *
Resample( filter_t *p_filter, block_t *p_in )
{
filter_sys_t *p_sys = p_filter->p_sys;
const vlc_tick_t i_pts = p_in->i_pts;

if( p_sys->vr_soxr )
{




soxr_t soxr;
block_t *p_flushed_out = NULL, *p_out = NULL;
const double f_ratio = p_filter->fmt_out.audio.i_rate
/ (double) p_filter->fmt_in.audio.i_rate;
const size_t i_olen = SoXR_GetOutLen( p_in->i_nb_samples, f_ratio );

if( f_ratio != p_sys->f_fixed_ratio )
{

soxr_set_io_ratio( p_sys->vr_soxr, 1 / f_ratio, i_olen );
soxr = p_sys->vr_soxr;
}
else if( f_ratio == 1.0f )
{

soxr = NULL;
p_out = p_in;
}
else
{

soxr = p_sys->soxr;
}


if( p_sys->last_soxr && soxr != p_sys->last_soxr && p_sys->i_last_olen )
{
p_flushed_out = SoXR_Resample( p_filter, p_sys->last_soxr,
NULL, p_sys->i_last_olen );
if( soxr )
msg_Dbg( p_filter, "Using '%s' engine", soxr_engine( soxr ) );
}

if( soxr )
{
assert( !p_out );
p_out = SoXR_Resample( p_filter, soxr, p_in, i_olen );
if( !p_out )
goto error;
}

if( p_flushed_out )
{

const unsigned i_nb_samples = p_flushed_out->i_nb_samples
+ p_out->i_nb_samples;

block_ChainAppend( &p_flushed_out, p_out );
p_out = block_ChainGather( p_flushed_out );
if( !p_out )
goto error;
p_out->i_nb_samples = i_nb_samples;
}
p_out->i_pts = i_pts;
return p_out;
}
else
{


const size_t i_olen = SoXR_GetOutLen( p_in->i_nb_samples,
p_sys->f_fixed_ratio );
block_t *p_out = SoXR_Resample( p_filter, p_sys->soxr, p_in, i_olen );
if( p_out )
p_out->i_pts = i_pts;
return p_out;
}
error:
block_Release( p_in );
return NULL;
}

static block_t *
Drain( filter_t *p_filter )
{
filter_sys_t *p_sys = p_filter->p_sys;

if( p_sys->last_soxr && p_sys->i_last_olen )
return SoXR_Resample( p_filter, p_sys->last_soxr, NULL,
p_sys->i_last_olen );
else
return NULL;
}

static void
Flush( filter_t *p_filter )
{
filter_sys_t *p_sys = p_filter->p_sys;

if( p_sys->last_soxr )
{
soxr_clear( p_sys->last_soxr );
p_sys->i_last_olen = 0;
p_sys->last_soxr = NULL;
}
}
