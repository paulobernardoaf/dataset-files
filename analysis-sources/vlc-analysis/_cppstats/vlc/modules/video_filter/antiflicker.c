#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdatomic.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
static int GetLuminanceAvg( picture_t * p_pic );
static picture_t *Filter( filter_t *, picture_t * );
static int AntiFlickerCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval,
void *p_data );
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
#define WINDOW_TEXT N_("Window size")
#define WINDOW_LONGTEXT N_("Number of frames (0 to 100)")
#define SFTN_TEXT N_("Softening value")
#define SFTN_LONGTEXT N_("Number of frames consider for smoothening (0 to 30)")
#define FILTER_PREFIX "antiflicker-"
#define MAX_WINDOW_SZ 100
#define MAX_SOFTENING_SZ 31
#define SCENE_CHANGE_THRESHOLD 100
vlc_module_begin ()
set_description( N_("Antiflicker video filter") )
set_shortname( N_( "antiflicker" ))
set_capability( "video filter", 0 )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
add_integer_with_range( FILTER_PREFIX "window-size", 10, 0, MAX_WINDOW_SZ,
WINDOW_TEXT, WINDOW_LONGTEXT, false )
add_integer_with_range( FILTER_PREFIX "softening-size", 10, 0, MAX_SOFTENING_SZ,
SFTN_TEXT, SFTN_LONGTEXT, false )
add_shortcut( "antiflicker" )
set_callbacks( Create, Destroy )
vlc_module_end ()
typedef struct
{
atomic_int i_window_size;
atomic_int i_softening;
int ia_luminance_data[MAX_WINDOW_SZ];
uint8_t *p_old_data;
} filter_sys_t;
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
switch( p_filter->fmt_in.video.i_chroma )
{
CASE_PLANAR_YUV
assert(p_filter->vctx_in == NULL);
break;
default:
msg_Err( p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_filter->fmt_in.video.i_chroma) );
return VLC_EGENERIC;
}
filter_sys_t *p_sys = malloc( sizeof( filter_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_filter->p_sys = p_sys;
p_filter->pf_video_filter = Filter;
atomic_init( &p_sys->i_window_size,
var_CreateGetIntegerCommand( p_filter,
FILTER_PREFIX"window-size" ) );
atomic_init( &p_sys->i_softening,
var_CreateGetIntegerCommand( p_filter,
FILTER_PREFIX"softening-size" ) );
p_sys->p_old_data = calloc( p_filter->fmt_in.video.i_width *
(p_filter->fmt_in.video.i_height+1),sizeof(*p_sys->p_old_data) );
if( p_sys->p_old_data == NULL )
{
free( p_sys );
return VLC_ENOMEM;
}
memset( p_sys->ia_luminance_data, 0,
sizeof(p_sys->ia_luminance_data) );
p_sys->ia_luminance_data[p_sys->i_window_size - 1] = 256;
var_AddCallback(p_filter,FILTER_PREFIX "window-size",
AntiFlickerCallback, p_sys);
var_AddCallback(p_filter,FILTER_PREFIX "softening-size",
AntiFlickerCallback, p_sys);
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
var_DelCallback(p_filter,FILTER_PREFIX "window-size",
AntiFlickerCallback, p_sys);
var_DelCallback(p_filter,FILTER_PREFIX "softening-size",
AntiFlickerCallback, p_sys);
free( p_sys->p_old_data );
free( p_sys );
}
static int GetLuminanceAvg( picture_t *p_pic )
{
uint8_t *p_yplane_out = p_pic->p[Y_PLANE].p_pixels;
int i_num_lines = p_pic->p[Y_PLANE].i_visible_lines;
int i_num_cols = p_pic->p[Y_PLANE].i_visible_pitch;
int i_in_pitch = p_pic->p[Y_PLANE].i_pitch;
if( i_num_lines == 0 || i_num_cols == 0 )
return 0;
unsigned lum_sum = 0;
for( int i_line = 0 ; i_line < i_num_lines ; ++i_line )
{
for( int i_col = 0 ; i_col < i_num_cols; ++i_col )
{
lum_sum += p_yplane_out[i_line*i_in_pitch+i_col];
}
}
unsigned div = i_num_lines * i_num_cols;
return (lum_sum + (div>>1)) / div;
}
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
if( !p_pic ) return NULL;
picture_t *p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
picture_Release( p_pic );
return NULL;
}
filter_sys_t *p_sys = p_filter->p_sys;
int i_window_size = atomic_load( &p_sys->i_window_size );
int i_softening = atomic_load( &p_sys->i_softening );
uint8_t *p_yplane_in = p_pic->p[Y_PLANE].p_pixels;
uint8_t *p_yplane_out = p_outpic->p[Y_PLANE].p_pixels;
bool scene_changed = false;
int i_num_lines = p_pic->p[Y_PLANE].i_visible_lines;
int i_num_cols = p_pic->p[Y_PLANE].i_visible_pitch;
int i_in_pitch = p_pic->p[Y_PLANE].i_pitch;
int i_out_pitch = p_outpic->p[Y_PLANE].i_pitch;
int lum_avg = GetLuminanceAvg(p_pic);
if( abs(lum_avg - p_sys->
ia_luminance_data[i_window_size - 1]) > SCENE_CHANGE_THRESHOLD
|| p_sys->ia_luminance_data[i_window_size - 1] == 256)
{
scene_changed = true;
}
if ( scene_changed )
{
for (int i = 0; i < i_window_size; ++i)
p_sys->ia_luminance_data[i] = lum_avg;
plane_CopyPixels( &p_outpic->p[Y_PLANE], &p_pic->p[Y_PLANE] );
}
else
{
for (int i = 0; i < i_window_size-1 ; ++i)
p_sys->ia_luminance_data[i] =
p_sys->ia_luminance_data[i+1];
p_sys->ia_luminance_data[i_window_size - 1] = lum_avg;
float scale = 1.0;
if (lum_avg > 0)
{
float filt = 0;
for (int i = 0; i < i_window_size; i++)
filt += (float) p_sys->ia_luminance_data[i];
scale = filt/(i_window_size*lum_avg);
}
uint8_t shift = 8;
int scale_num = __MIN(scale,255) * ( 1 << shift );
for( int i_line = 0 ; i_line < i_num_lines ; i_line++ )
{
for( int i_col = 0; i_col < i_num_cols ; i_col++ )
{
uint8_t pixel_data = p_yplane_in[i_line*i_in_pitch+i_col];
int pixel_val = ( scale_num * pixel_data +
(1<<(shift -1)) ) >> shift;
p_yplane_out[i_line*i_out_pitch+i_col] =
(pixel_val>255) ? 255:pixel_val;
}
}
}
plane_CopyPixels( &p_outpic->p[U_PLANE], &p_pic->p[U_PLANE] );
plane_CopyPixels( &p_outpic->p[V_PLANE], &p_pic->p[V_PLANE] );
if (scene_changed || i_softening == 0)
{
return CopyInfoAndRelease( p_outpic, p_pic );
}
uint8_t *p_yplane_out_old = p_sys->p_old_data;
int i_video_width = p_filter->fmt_in.video.i_width;
for( int i_line = 0 ; i_line < i_num_lines ; i_line++ )
{
for( int i_col = 0; i_col < i_num_cols ; i_col++ )
{
uint8_t pixel_data = p_yplane_out[i_line*i_out_pitch+i_col];
uint8_t pixel_old = p_yplane_out_old[i_line*i_video_width+i_col];
int diff = abs(pixel_data - pixel_old);
if (diff < i_softening)
{
if (diff > (i_softening >> 1))
{
p_yplane_out_old[i_line*i_video_width+i_col] =
((pixel_data * 2) + pixel_old) /3;
}
}
else
{
p_yplane_out_old[i_line*i_video_width+i_col] = pixel_data;
}
p_yplane_out[i_line*i_out_pitch+i_col] =
p_yplane_out_old[i_line*i_video_width+i_col];
}
}
return CopyInfoAndRelease( p_outpic, p_pic );
}
static int AntiFlickerCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval,
void *p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(oldval);
filter_sys_t *p_sys = (filter_sys_t *)p_data;
if( !strcmp( psz_var, FILTER_PREFIX "window-size" ) )
atomic_store( &p_sys->i_window_size, newval.i_int );
else if( !strcmp( psz_var, FILTER_PREFIX "softening-size" ) )
atomic_store( &p_sys->i_softening, newval.i_int );
return VLC_SUCCESS;
}
