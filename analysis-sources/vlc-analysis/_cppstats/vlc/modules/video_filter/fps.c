#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
static int Open( vlc_object_t *p_this);
static void Close( vlc_object_t *p_this);
static picture_t *Filter( filter_t *p_filter, picture_t *p_picture);
#define CFG_PREFIX "fps-"
#define FPS_TEXT N_( "Frame rate" )
vlc_module_begin ()
set_description( N_("FPS conversion video filter") )
set_shortname( N_("FPS Converter" ))
set_capability( "video filter", 0 )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
add_shortcut( "fps" )
add_string( CFG_PREFIX "fps", NULL, FPS_TEXT, FPS_TEXT, false )
set_callbacks( Open, Close )
vlc_module_end ()
static const char *const ppsz_filter_options[] = {
"fps",
NULL
};
typedef struct
{
date_t next_output_pts; 
picture_t *p_previous_pic;
vlc_tick_t i_output_frame_interval;
} filter_sys_t;
static picture_t *Filter( filter_t *p_filter, picture_t *p_picture)
{
filter_sys_t *p_sys = p_filter->p_sys;
if( unlikely( p_picture->date == VLC_TICK_INVALID) )
{
msg_Dbg( p_filter, "skipping non-dated picture");
picture_Release( p_picture );
return NULL;
}
p_picture->format.i_frame_rate = p_filter->fmt_out.video.i_frame_rate;
p_picture->format.i_frame_rate_base = p_filter->fmt_out.video.i_frame_rate_base;
if( unlikely( ( date_Get( &p_sys->next_output_pts ) == VLC_TICK_INVALID ) ||
( p_picture->date > ( date_Get( &p_sys->next_output_pts ) + p_sys->i_output_frame_interval ) )
) )
{
msg_Dbg( p_filter, "Resetting timestamps" );
date_Set( &p_sys->next_output_pts, p_picture->date );
if( p_sys->p_previous_pic )
picture_Release( p_sys->p_previous_pic );
p_sys->p_previous_pic = picture_Hold( p_picture );
date_Increment( &p_sys->next_output_pts, 1 );
return p_picture;
}
if( p_picture->date <
( date_Get( &p_sys->next_output_pts ) - p_sys->i_output_frame_interval ) )
{
if( p_sys->p_previous_pic )
picture_Release( p_sys->p_previous_pic );
p_sys->p_previous_pic = p_picture;
return NULL;
}
p_sys->p_previous_pic->date = date_Get( &p_sys->next_output_pts );
date_Increment( &p_sys->next_output_pts, 1 );
picture_t *last_pic = p_sys->p_previous_pic;
while( unlikely( (date_Get( &p_sys->next_output_pts ) + p_sys->i_output_frame_interval ) < p_picture->date ) )
{
picture_t *p_tmp = NULL;
p_tmp = picture_NewFromFormat( &p_filter->fmt_out.video );
picture_Copy( p_tmp, p_sys->p_previous_pic);
p_tmp->date = date_Get( &p_sys->next_output_pts );
p_tmp->p_next = NULL;
last_pic->p_next = p_tmp;
last_pic = p_tmp;
date_Increment( &p_sys->next_output_pts, 1 );
}
last_pic = p_sys->p_previous_pic;
p_sys->p_previous_pic = p_picture;
return last_pic;
}
static int Open( vlc_object_t *p_this)
{
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys;
p_sys = p_filter->p_sys = malloc( sizeof( *p_sys ) );
if( unlikely( !p_sys ) )
return VLC_ENOMEM;
config_ChainParse( p_filter, CFG_PREFIX, ppsz_filter_options,
p_filter->p_cfg );
const unsigned int i_out_frame_rate = p_filter->fmt_out.video.i_frame_rate;
const unsigned int i_out_frame_rate_base = p_filter->fmt_out.video.i_frame_rate_base;
video_format_Clean( &p_filter->fmt_out.video );
video_format_Copy( &p_filter->fmt_out.video, &p_filter->fmt_in.video );
if( var_InheritURational( p_filter, &p_filter->fmt_out.video.i_frame_rate,
&p_filter->fmt_out.video.i_frame_rate_base, CFG_PREFIX "fps" ) )
{
p_filter->fmt_out.video.i_frame_rate = i_out_frame_rate;
p_filter->fmt_out.video.i_frame_rate_base = i_out_frame_rate_base;
}
if( p_filter->fmt_out.video.i_frame_rate == 0 ) {
msg_Err( p_filter, "Invalid output frame rate" );
free( p_sys );
return VLC_EGENERIC;
}
msg_Dbg( p_filter, "Converting fps from %d/%d -> %d/%d",
p_filter->fmt_in.video.i_frame_rate, p_filter->fmt_in.video.i_frame_rate_base,
p_filter->fmt_out.video.i_frame_rate, p_filter->fmt_out.video.i_frame_rate_base );
p_sys->i_output_frame_interval = vlc_tick_from_samples(p_filter->fmt_out.video.i_frame_rate_base,
p_filter->fmt_out.video.i_frame_rate);
date_Init( &p_sys->next_output_pts,
p_filter->fmt_out.video.i_frame_rate, p_filter->fmt_out.video.i_frame_rate_base );
p_sys->p_previous_pic = NULL;
p_filter->pf_video_filter = Filter;
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
if( p_sys->p_previous_pic )
picture_Release( p_sys->p_previous_pic );
free( p_sys );
}
