#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_aout_volume.h>
#include <vlc_filter.h>
#include <vlc_modules.h>
#include <vlc_plugin.h>
static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );
static block_t *Process ( filter_t *, block_t * );
typedef struct
{
audio_volume_t volume;
float f_gain;
module_t *module;
} filter_sys_t;
#define GAIN_VALUE_TEXT N_( "Gain multiplier" )
#define GAIN_VALUE_LONGTEXT N_( "Increase or decrease the gain (default 1.0)" )
vlc_module_begin()
set_shortname( N_("Gain") )
set_description( N_("Gain control filter") )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AFILTER )
add_float( "gain-value", 1.0, GAIN_VALUE_TEXT,
GAIN_VALUE_LONGTEXT, false )
set_capability( "audio filter", 0 )
set_callbacks( Open, Close )
vlc_module_end()
static int Open( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = vlc_object_create( p_this, sizeof( *p_sys ) );
if( unlikely( p_sys == NULL ) )
return VLC_ENOMEM;
p_filter->p_sys = p_sys;
p_sys->volume.format = p_filter->fmt_in.audio.i_format;
p_sys->module = module_need( &p_sys->volume, "audio volume", NULL, false );
if( p_sys->module == NULL )
{
msg_Warn( p_filter, "unsupported format" );
vlc_object_delete(&p_sys->volume);
return VLC_EGENERIC;
}
p_sys->f_gain = var_InheritFloat( vlc_object_parent(p_filter),
"gain-value" );
msg_Dbg( p_filter, "gain multiplier sets to %.2fx", p_sys->f_gain );
p_filter->fmt_out.audio = p_filter->fmt_in.audio;
p_filter->pf_audio_filter = Process;
return VLC_SUCCESS;
}
static block_t *Process( filter_t *p_filter, block_t *p_block )
{
filter_sys_t *p_sys = p_filter->p_sys;
p_sys->volume.amplify( &p_sys->volume, p_block, p_sys->f_gain );
return p_block;
}
static void Close( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
module_unneed( &p_sys->volume, p_sys->module );
vlc_object_delete(&p_sys->volume);
}
