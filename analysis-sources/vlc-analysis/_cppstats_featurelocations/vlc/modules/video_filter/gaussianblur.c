

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"

#include <math.h> 




static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );

#define SIGMA_MIN (0.01)
#define SIGMA_MAX (4096.0)

#define SIGMA_TEXT N_("Gaussian's std deviation")
#define SIGMA_LONGTEXT N_( "Gaussian's standard deviation. The blurring will take " "into account pixels up to 3*sigma away in any direction.")



#define GAUSSIAN_HELP N_("Add a blurring effect")

#define FILTER_PREFIX "gaussianblur-"

vlc_module_begin ()
set_description( N_("Gaussian blur video filter") )
set_shortname( N_( "Gaussian Blur" ))
set_help(GAUSSIAN_HELP)
set_capability( "video filter", 0 )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )

add_float_with_range( FILTER_PREFIX "sigma", 2., SIGMA_MIN, SIGMA_MAX,
SIGMA_TEXT, SIGMA_LONGTEXT,
false )

set_callbacks( Create, Destroy )
vlc_module_end ()




static picture_t *Filter( filter_t *, picture_t * );

static const char *const ppsz_filter_options[] = {
"sigma", NULL
};






#define DONT_USE_FLOATS

#if defined(DONT_USE_FLOATS)
#define type_t int
#else
#define type_t float
#endif

typedef struct
{
double f_sigma;
int i_dim;

type_t *pt_distribution;
type_t *pt_buffer;
type_t *pt_scale;
} filter_sys_t;

static void gaussianblur_InitDistribution( filter_sys_t *p_sys )
{
double f_sigma = p_sys->f_sigma;
int i_dim = (int)(3.*f_sigma);
type_t *pt_distribution = xmalloc( (2*i_dim+1) * sizeof( type_t ) );

for( int x = -i_dim; x <= i_dim; x++ )
{
const float f_distribution = sqrt( exp(-(x*x)/(f_sigma*f_sigma) ) / (2.*M_PI*f_sigma*f_sigma) );
#if defined(DONT_USE_FLOATS)
const float f_factor = 1 << 8;
#else
const float f_factor = 1;
#endif

pt_distribution[i_dim+x] = (type_t)( f_distribution * f_factor );

}
p_sys->i_dim = i_dim;
p_sys->pt_distribution = pt_distribution;
}

static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;

if( p_filter->fmt_in.video.i_chroma != VLC_CODEC_I420
&& p_filter->fmt_in.video.i_chroma != VLC_CODEC_J420
&& p_filter->fmt_in.video.i_chroma != VLC_CODEC_YV12

&& p_filter->fmt_in.video.i_chroma != VLC_CODEC_I422
&& p_filter->fmt_in.video.i_chroma != VLC_CODEC_J422
)
{

msg_Err( p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_filter->fmt_in.video.i_chroma) );
return VLC_EGENERIC;
}

if( p_filter->fmt_in.video.i_chroma != p_filter->fmt_out.video.i_chroma )
{
msg_Err( p_filter, "Input and output chromas don't match" );
return VLC_EGENERIC;
}

filter_sys_t *p_sys = malloc( sizeof( filter_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_filter->p_sys = p_sys;

config_ChainParse( p_filter, FILTER_PREFIX, ppsz_filter_options,
p_filter->p_cfg );

p_filter->pf_video_filter = Filter;

p_sys->f_sigma =
var_CreateGetFloat( p_filter, FILTER_PREFIX "sigma" );
if( p_sys->f_sigma <= 0. )
{
msg_Err( p_filter, "sigma must be greater than zero" );
return VLC_EGENERIC;
}
gaussianblur_InitDistribution( p_sys );
msg_Dbg( p_filter, "gaussian distribution is %d pixels wide",
p_sys->i_dim*2+1 );

p_sys->pt_buffer = NULL;
p_sys->pt_scale = NULL;

return VLC_SUCCESS;
}

static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;

free( p_sys->pt_distribution );
free( p_sys->pt_buffer );
free( p_sys->pt_scale );

free( p_sys );
}

static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;
filter_sys_t *p_sys = p_filter->p_sys;
const int i_dim = p_sys->i_dim;
type_t *pt_buffer;
type_t *pt_scale;
const type_t *pt_distribution = p_sys->pt_distribution;

if( !p_pic ) return NULL;

p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
picture_Release( p_pic );
return NULL;
}
if( !p_sys->pt_buffer )
{
p_sys->pt_buffer = realloc_or_free( p_sys->pt_buffer,
p_pic->p[Y_PLANE].i_visible_lines *
p_pic->p[Y_PLANE].i_pitch * sizeof( type_t ) );
}

pt_buffer = p_sys->pt_buffer;
if( !p_sys->pt_scale )
{
const int i_visible_lines = p_pic->p[Y_PLANE].i_visible_lines;
const int i_visible_pitch = p_pic->p[Y_PLANE].i_visible_pitch;
const int i_pitch = p_pic->p[Y_PLANE].i_pitch;

p_sys->pt_scale = xmalloc( i_visible_lines * i_pitch * sizeof( type_t ) );
pt_scale = p_sys->pt_scale;

for( int i_line = 0; i_line < i_visible_lines; i_line++ )
{
for( int i_col = 0; i_col < i_visible_pitch; i_col++ )
{
type_t t_value = 0;

for( int y = __MAX( -i_dim, -i_line );
y <= __MIN( i_dim, i_visible_lines - i_line - 1 );
y++ )
{
for( int x = __MAX( -i_dim, -i_col );
x <= __MIN( i_dim, i_visible_pitch - i_col + 1 );
x++ )
{
t_value += pt_distribution[y+i_dim] *
pt_distribution[x+i_dim];
}
}
pt_scale[i_line*i_pitch+i_col] = t_value;
}
}
}

pt_scale = p_sys->pt_scale;
for( int i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
{

uint8_t *p_in = p_pic->p[i_plane].p_pixels;
uint8_t *p_out = p_outpic->p[i_plane].p_pixels;

const int i_visible_lines = p_pic->p[i_plane].i_visible_lines;
const int i_visible_pitch = p_pic->p[i_plane].i_visible_pitch;
const int i_in_pitch = p_pic->p[i_plane].i_pitch;

const int x_factor = p_pic->p[Y_PLANE].i_visible_pitch/i_visible_pitch-1;
const int y_factor = p_pic->p[Y_PLANE].i_visible_lines/i_visible_lines-1;

for( int i_line = 0; i_line < i_visible_lines; i_line++ )
{
for( int i_col = 0; i_col < i_visible_pitch; i_col++ )
{
type_t t_value = 0;
const int c = i_line*i_in_pitch+i_col;
for( int x = __MAX( -i_dim, -i_col*(x_factor+1) );
x <= __MIN( i_dim, (i_visible_pitch - i_col)*(x_factor+1) + 1 );
x++ )
{
t_value += pt_distribution[x+i_dim] *
p_in[c+(x>>x_factor)];
}
pt_buffer[c] = t_value;
}
}
for( int i_line = 0; i_line < i_visible_lines; i_line++ )
{
for( int i_col = 0; i_col < i_visible_pitch; i_col++ )
{
type_t t_value = 0;
const int c = i_line*i_in_pitch+i_col;
for( int y = __MAX( -i_dim, (-i_line)*(y_factor+1) );
y <= __MIN( i_dim, (i_visible_lines - i_line)*(y_factor+1) - 1 );
y++ )
{
t_value += pt_distribution[y+i_dim] *
pt_buffer[c+(y>>y_factor)*i_in_pitch];
}

const type_t t_scale = pt_scale[(i_line<<y_factor)*(i_in_pitch<<x_factor)+(i_col<<x_factor)];
p_out[i_line * p_outpic->p[i_plane].i_pitch + i_col] = (uint8_t)(t_value / t_scale); 
}
}
}

return CopyInfoAndRelease( p_outpic, p_pic );
}
