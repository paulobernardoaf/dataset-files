#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h> 
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
enum { GRADIENT, EDGE, HOUGH };
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t *Filter( filter_t *, picture_t * );
static int GradientCallback( vlc_object_t *, char const *,
vlc_value_t, vlc_value_t,
void * );
static void FilterGradient( filter_t *, picture_t *, picture_t * );
static void FilterEdge ( filter_t *, picture_t *, picture_t * );
static void FilterHough ( filter_t *, picture_t *, picture_t * );
#define MODE_TEXT N_("Distort mode")
#define MODE_LONGTEXT N_("Distort mode, one of \"gradient\", \"edge\" and \"hough\".")
#define GRADIENT_TEXT N_("Gradient image type")
#define GRADIENT_LONGTEXT N_("Gradient image type (0 or 1). 0 will " "turn the image to white while 1 will keep colors." )
#define CARTOON_TEXT N_("Apply cartoon effect")
#define CARTOON_LONGTEXT N_("Apply cartoon effect. It is only used by " "\"gradient\" and \"edge\".")
#define GRADIENT_HELP N_("Apply color gradient or edge detection effects")
static const char *const mode_list[] = { "gradient", "edge", "hough" };
static const char *const mode_list_text[] = { N_("Gradient"), N_("Edge"), N_("Hough") };
#define FILTER_PREFIX "gradient-"
vlc_module_begin ()
set_description( N_("Gradient video filter") )
set_shortname( N_( "Gradient" ))
set_help(GRADIENT_HELP)
set_capability( "video filter", 0 )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
add_string( FILTER_PREFIX "mode", "gradient",
MODE_TEXT, MODE_LONGTEXT, false )
change_string_list( mode_list, mode_list_text )
add_integer_with_range( FILTER_PREFIX "type", 0, 0, 1,
GRADIENT_TEXT, GRADIENT_LONGTEXT, false )
add_bool( FILTER_PREFIX "cartoon", true,
CARTOON_TEXT, CARTOON_LONGTEXT, false )
add_shortcut( "gradient" )
set_callbacks( Create, Destroy )
vlc_module_end ()
static const char *const ppsz_filter_options[] = {
"mode", "type", "cartoon", NULL
};
typedef struct
{
vlc_mutex_t lock;
int i_mode;
int i_gradient_type;
bool b_cartoon;
uint32_t *p_buf32;
uint32_t *p_buf32_bis;
uint8_t *p_buf8;
int *p_pre_hough;
} filter_sys_t;
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
char *psz_method;
switch( p_filter->fmt_in.video.i_chroma )
{
CASE_PLANAR_YUV
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
p_sys->p_pre_hough = NULL;
config_ChainParse( p_filter, FILTER_PREFIX, ppsz_filter_options,
p_filter->p_cfg );
if( !(psz_method =
var_CreateGetNonEmptyStringCommand( p_filter, FILTER_PREFIX "mode" )) )
{
msg_Err( p_filter, "configuration variable "
FILTER_PREFIX "mode empty" );
p_sys->i_mode = GRADIENT;
}
else
{
if( !strcmp( psz_method, "gradient" ) )
{
p_sys->i_mode = GRADIENT;
}
else if( !strcmp( psz_method, "edge" ) )
{
p_sys->i_mode = EDGE;
}
else if( !strcmp( psz_method, "hough" ) )
{
p_sys->i_mode = HOUGH;
}
else
{
msg_Err( p_filter, "no valid gradient mode provided (%s)", psz_method );
p_sys->i_mode = GRADIENT;
}
}
free( psz_method );
p_sys->i_gradient_type =
var_CreateGetIntegerCommand( p_filter, FILTER_PREFIX "type" );
p_sys->b_cartoon =
var_CreateGetBoolCommand( p_filter, FILTER_PREFIX "cartoon" );
vlc_mutex_init( &p_sys->lock );
var_AddCallback( p_filter, FILTER_PREFIX "mode",
GradientCallback, p_sys );
var_AddCallback( p_filter, FILTER_PREFIX "type",
GradientCallback, p_sys );
var_AddCallback( p_filter, FILTER_PREFIX "cartoon",
GradientCallback, p_sys );
p_sys->p_buf32 = NULL;
p_sys->p_buf32_bis = NULL;
p_sys->p_buf8 = NULL;
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
var_DelCallback( p_filter, FILTER_PREFIX "mode",
GradientCallback, p_sys );
var_DelCallback( p_filter, FILTER_PREFIX "type",
GradientCallback, p_sys );
var_DelCallback( p_filter, FILTER_PREFIX "cartoon",
GradientCallback, p_sys );
free( p_sys->p_buf32 );
free( p_sys->p_buf32_bis );
free( p_sys->p_buf8 );
free( p_sys->p_pre_hough );
free( p_sys );
}
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;
if( !p_pic ) return NULL;
p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
picture_Release( p_pic );
return NULL;
}
filter_sys_t *p_sys = p_filter->p_sys;
vlc_mutex_lock( &p_sys->lock );
switch( p_sys->i_mode )
{
case EDGE:
FilterEdge( p_filter, p_pic, p_outpic );
break;
case GRADIENT:
FilterGradient( p_filter, p_pic, p_outpic );
break;
case HOUGH:
FilterHough( p_filter, p_pic, p_outpic );
break;
default:
break;
}
vlc_mutex_unlock( &p_sys->lock );
return CopyInfoAndRelease( p_outpic, p_pic );
}
static void GaussianConvolution( picture_t *p_inpic, uint32_t *p_smooth )
{
const uint8_t *p_inpix = p_inpic->p[Y_PLANE].p_pixels;
const int i_src_pitch = p_inpic->p[Y_PLANE].i_pitch;
const int i_src_visible = p_inpic->p[Y_PLANE].i_visible_pitch;
const int i_num_lines = p_inpic->p[Y_PLANE].i_visible_lines;
for( int y = 2; y < i_num_lines - 2; y++ )
{
for( int x = 2; x < i_src_visible - 2; x++ )
{
p_smooth[y*i_src_visible+x] = (uint32_t)(
( p_inpix[(y-2)*i_src_pitch+x-2] )
+ ((p_inpix[(y-2)*i_src_pitch+x-1]
+ p_inpix[(y-2)*i_src_pitch+x]
+ p_inpix[(y-2)*i_src_pitch+x+1])<<1 )
+ ( p_inpix[(y-2)*i_src_pitch+x+2] )
+ ((p_inpix[(y-1)*i_src_pitch+x-2]
+ ( p_inpix[(y-1)*i_src_pitch+x-1]<<1 )
+ ( p_inpix[(y-1)*i_src_pitch+x]*3 )
+ ( p_inpix[(y-1)*i_src_pitch+x+1]<<1 )
+ p_inpix[(y-1)*i_src_pitch+x+2]
+ p_inpix[y*i_src_pitch+x-2]
+ ( p_inpix[y*i_src_pitch+x-1]*3 )
+ ( p_inpix[y*i_src_pitch+x]<<2 )
+ ( p_inpix[y*i_src_pitch+x+1]*3 )
+ p_inpix[y*i_src_pitch+x+2]
+ p_inpix[(y+1)*i_src_pitch+x-2]
+ ( p_inpix[(y+1)*i_src_pitch+x-1]<<1 )
+ ( p_inpix[(y+1)*i_src_pitch+x]*3 )
+ ( p_inpix[(y+1)*i_src_pitch+x+1]<<1 )
+ p_inpix[(y+1)*i_src_pitch+x+2] )<<1 )
+ ( p_inpix[(y+2)*i_src_pitch+x-2] )
+ ((p_inpix[(y+2)*i_src_pitch+x-1]
+ p_inpix[(y+2)*i_src_pitch+x]
+ p_inpix[(y+2)*i_src_pitch+x+1])<<1 )
+ ( p_inpix[(y+2)*i_src_pitch+x+2] )
) >> 6 ;
}
}
}
static void FilterGradient( filter_t *p_filter, picture_t *p_inpic,
picture_t *p_outpic )
{
filter_sys_t *p_sys = p_filter->p_sys;
const int i_src_pitch = p_inpic->p[Y_PLANE].i_pitch;
const int i_src_visible = p_inpic->p[Y_PLANE].i_visible_pitch;
const int i_dst_pitch = p_outpic->p[Y_PLANE].i_pitch;
const int i_num_lines = p_inpic->p[Y_PLANE].i_visible_lines;
const uint8_t *p_inpix = p_inpic->p[Y_PLANE].p_pixels;
uint8_t *p_outpix = p_outpic->p[Y_PLANE].p_pixels;
uint32_t *p_smooth;
if( !p_sys->p_buf32 )
p_sys->p_buf32 =
vlc_alloc( i_num_lines * i_src_visible, sizeof(uint32_t));
p_smooth = p_sys->p_buf32;
if( !p_smooth ) return;
if( p_sys->b_cartoon )
{
plane_CopyPixels( &p_outpic->p[U_PLANE], &p_inpic->p[U_PLANE] );
plane_CopyPixels( &p_outpic->p[V_PLANE], &p_inpic->p[V_PLANE] );
}
else
{
memset( p_outpic->p[U_PLANE].p_pixels, 0x80,
p_outpic->p[U_PLANE].i_lines * p_outpic->p[U_PLANE].i_pitch );
memset( p_outpic->p[V_PLANE].p_pixels, 0x80,
p_outpic->p[V_PLANE].i_lines * p_outpic->p[V_PLANE].i_pitch );
}
GaussianConvolution( p_inpic, p_smooth );
#define FOR for( int y = 1; y < i_num_lines - 1; y++ ) { for( int x = 1; x < i_src_visible - 1; x++ ) { const uint32_t a = abs(((int)p_smooth[(y - 1) * i_src_visible + x - 1] - (int)p_smooth[(y + 1) * i_src_visible + x - 1]) + (((int)p_smooth[(y - 1) * i_src_visible + x] - (int)p_smooth[(y + 1) * i_src_visible + x]) * 2) + ((int)p_smooth[(y - 1) * i_src_visible + x + 1] - (int)p_smooth[(y + 1) * i_src_visible + x + 1])) + abs(((int)p_smooth[(y - 1) * i_src_visible + x - 1] - (int)p_smooth[(y - 1) * i_src_visible + x + 1]) + (((int)p_smooth[y * i_src_visible + x - 1] - (int)p_smooth[y * i_src_visible + x + 1]) * 2) + ((int)p_smooth[(y + 1) * i_src_visible + x - 1] - (int)p_smooth[(y + 1) * i_src_visible + x + 1]));
if( p_sys->i_gradient_type )
{
if( p_sys->b_cartoon )
{
FOR
if( a > 60 )
{
p_outpix[y*i_dst_pitch+x] = 0x00;
}
else
{
if( p_smooth[y*i_src_visible+x] > 0xa0 )
p_outpix[y*i_dst_pitch+x] =
0xff - ((0xff - p_inpix[y*i_src_pitch+x] )>>2);
else if( p_smooth[y*i_src_visible+x] > 0x70 )
p_outpix[y*i_dst_pitch+x] =
0xa0 - ((0xa0 - p_inpix[y*i_src_pitch+x] )>>2);
else if( p_smooth[y*i_src_visible+x] > 0x28 )
p_outpix[y*i_dst_pitch+x] =
0x70 - ((0x70 - p_inpix[y*i_src_pitch+x] )>>2);
else
p_outpix[y*i_dst_pitch+x] =
0x28 - ((0x28 - p_inpix[y*i_src_pitch+x] )>>2);
}
}}
}
else
{
FOR
p_outpix[y*i_dst_pitch+x] = clip_uint8_vlc( a );
}}
}
}
else
{
FOR
if( a>>8 )
p_outpix[y*i_dst_pitch+x] = 0;
else
p_outpix[y*i_dst_pitch+x] = 0xff-(uint8_t)a;
}}
}
#undef FOR
}
#define THETA_Y 0
#define THETA_X 1
#define THETA_P 2
#define THETA_M 3
static void FilterEdge( filter_t *p_filter, picture_t *p_inpic,
picture_t *p_outpic )
{
filter_sys_t *p_sys = p_filter->p_sys;
const int i_src_pitch = p_inpic->p[Y_PLANE].i_pitch;
const int i_src_visible = p_inpic->p[Y_PLANE].i_visible_pitch;
const int i_dst_pitch = p_outpic->p[Y_PLANE].i_pitch;
const int i_num_lines = p_inpic->p[Y_PLANE].i_visible_lines;
const uint8_t *p_inpix = p_inpic->p[Y_PLANE].p_pixels;
uint8_t *p_outpix = p_outpic->p[Y_PLANE].p_pixels;
uint32_t *p_smooth;
uint32_t *p_grad;
uint8_t *p_theta;
if( !p_sys->p_buf32 )
p_sys->p_buf32 =
vlc_alloc( i_num_lines * i_src_visible, sizeof(uint32_t));
p_smooth = p_sys->p_buf32;
if( !p_sys->p_buf32_bis )
p_sys->p_buf32_bis =
vlc_alloc( i_num_lines * i_src_visible, sizeof(uint32_t));
p_grad = p_sys->p_buf32_bis;
if( !p_sys->p_buf8 )
p_sys->p_buf8 =
vlc_alloc( i_num_lines * i_src_visible, sizeof(uint8_t));
p_theta = p_sys->p_buf8;
if( !p_smooth || !p_grad || !p_theta ) return;
if( p_sys->b_cartoon )
{
plane_CopyPixels( &p_outpic->p[U_PLANE], &p_inpic->p[U_PLANE] );
plane_CopyPixels( &p_outpic->p[V_PLANE], &p_inpic->p[V_PLANE] );
}
else
{
memset( p_outpic->p[Y_PLANE].p_pixels, 0xff,
p_outpic->p[Y_PLANE].i_lines * p_outpic->p[Y_PLANE].i_pitch );
memset( p_outpic->p[U_PLANE].p_pixels, 0x80,
p_outpic->p[U_PLANE].i_lines * p_outpic->p[U_PLANE].i_pitch );
memset( p_outpic->p[V_PLANE].p_pixels, 0x80,
p_outpic->p[V_PLANE].i_lines * p_outpic->p[V_PLANE].i_pitch );
}
GaussianConvolution( p_inpic, p_smooth );
for( int y = 1; y < i_num_lines - 1; y++ )
{
for( int x = 1; x < i_src_visible - 1; x++ )
{
const int gradx =
( p_smooth[(y-1)*i_src_visible+x-1]
- p_smooth[(y+1)*i_src_visible+x-1] )
+ ( ( p_smooth[(y-1)*i_src_visible+x]
- p_smooth[(y+1)*i_src_visible+x] ) <<1 )
+ ( p_smooth[(y-1)*i_src_visible+x+1]
- p_smooth[(y+1)*i_src_visible+x+1] );
const int grady =
( p_smooth[(y-1)*i_src_visible+x-1]
- p_smooth[(y-1)*i_src_visible+x+1] )
+ ( ( p_smooth[y*i_src_visible+x-1]
- p_smooth[y*i_src_visible+x+1] ) <<1 )
+ ( p_smooth[(y+1)*i_src_visible+x-1]
- p_smooth[(y+1)*i_src_visible+x+1] );
p_grad[y*i_src_visible+x] = (uint32_t)(abs( gradx ) + abs( grady ));
if( (grady<<1) > gradx )
p_theta[y*i_src_visible+x] = THETA_P;
else if( (grady<<1) < -gradx )
p_theta[y*i_src_visible+x] = THETA_M;
else if( !gradx || abs(grady) > abs(gradx)<<1 )
p_theta[y*i_src_visible+x] = THETA_Y;
else
p_theta[y*i_src_visible+x] = THETA_X;
}
}
for( int y = 1; y < i_num_lines - 1; y++ )
{
for( int x = 1; x < i_src_visible - 1; x++ )
{
if( p_grad[y*i_src_visible+x] > 40 )
{
switch( p_theta[y*i_src_visible+x] )
{
case THETA_Y:
if( p_grad[y*i_src_visible+x] > p_grad[(y-1)*i_src_visible+x]
&& p_grad[y*i_src_visible+x] > p_grad[(y+1)*i_src_visible+x] )
{
p_outpix[y*i_dst_pitch+x] = 0;
break;
} else goto colorize;
case THETA_P:
if( p_grad[y*i_src_visible+x] > p_grad[(y-1)*i_src_visible+x-1]
&& p_grad[y*i_src_visible+x] > p_grad[(y+1)*i_src_visible+x+1] )
{
p_outpix[y*i_dst_pitch+x] = 0;
break;
} else goto colorize;
case THETA_M:
if( p_grad[y*i_src_visible+x] > p_grad[(y-1)*i_src_visible+x+1]
&& p_grad[y*i_src_visible+x] > p_grad[(y+1)*i_src_visible+x-1] )
{
p_outpix[y*i_dst_pitch+x] = 0;
break;
} else goto colorize;
case THETA_X:
if( p_grad[y*i_src_visible+x] > p_grad[y*i_src_visible+x-1]
&& p_grad[y*i_src_visible+x] > p_grad[y*i_src_visible+x+1] )
{
p_outpix[y*i_dst_pitch+x] = 0;
break;
} else goto colorize;
}
}
else
{
colorize:
if( p_sys->b_cartoon )
{
if( p_smooth[y*i_src_visible+x] > 0xa0 )
p_outpix[y*i_dst_pitch+x] = (uint8_t)
0xff - ((0xff - p_inpix[y*i_src_pitch+x] )>>2);
else if( p_smooth[y*i_src_visible+x] > 0x70 )
p_outpix[y*i_dst_pitch+x] =(uint8_t)
0xa0 - ((0xa0 - p_inpix[y*i_src_pitch+x] )>>2);
else if( p_smooth[y*i_src_visible+x] > 0x28 )
p_outpix[y*i_dst_pitch+x] =(uint8_t)
0x70 - ((0x70 - p_inpix[y*i_src_pitch+x] )>>2);
else
p_outpix[y*i_dst_pitch+x] =(uint8_t)
0x28 - ((0x28 - p_inpix[y*i_src_pitch+x] )>>2);
}
}
}
}
}
#define p_pre_hough p_sys->p_pre_hough
static void FilterHough( filter_t *p_filter, picture_t *p_inpic,
picture_t *p_outpic )
{
filter_sys_t *p_sys = p_filter->p_sys;
int i_src_visible = p_inpic->p[Y_PLANE].i_visible_pitch;
int i_dst_pitch = p_outpic->p[Y_PLANE].i_pitch;
int i_num_lines = p_inpic->p[Y_PLANE].i_visible_lines;
uint8_t *p_outpix = p_outpic->p[Y_PLANE].p_pixels;
int i_diag = sqrt( i_num_lines * i_num_lines +
i_src_visible * i_src_visible);
int i_max, i_phi_max, i_rho, i_rho_max;
int i_nb_steps = 90;
double d_step = M_PI / i_nb_steps;
double d_sin;
double d_cos;
uint32_t *p_smooth;
int *p_hough = vlc_alloc( i_diag * i_nb_steps, sizeof(int) );
if( ! p_hough ) return;
p_smooth = vlc_alloc( i_num_lines * i_src_visible, sizeof(uint32_t));
if( !p_smooth )
{
free( p_hough );
return;
}
if( ! p_pre_hough )
{
msg_Dbg(p_filter, "Starting precalculation");
p_pre_hough = vlc_alloc( i_num_lines*i_src_visible*i_nb_steps,sizeof(int));
if( ! p_pre_hough )
{
free( p_smooth );
free( p_hough );
return;
}
for( int i = 0; i < i_nb_steps; i++)
{
d_sin = sin(d_step * i);
d_cos = cos(d_step * i);
for( int y = 0; y < i_num_lines; y++ )
for( int x = 0; x < i_src_visible; x++ )
{
p_pre_hough[(i*i_num_lines+y)*i_src_visible + x] =
ceil(x*d_sin + y*d_cos);
}
}
msg_Dbg(p_filter, "Precalculation done");
}
memset( p_hough, 0, i_diag * i_nb_steps * sizeof(int) );
plane_CopyPixels( &p_outpic->p[Y_PLANE], &p_inpic->p[Y_PLANE] );
plane_CopyPixels( &p_outpic->p[U_PLANE], &p_inpic->p[U_PLANE] );
plane_CopyPixels( &p_outpic->p[V_PLANE], &p_inpic->p[V_PLANE] );
GaussianConvolution( p_inpic, p_smooth );
i_max = 0;
i_rho_max = 0;
i_phi_max = 0;
for( int y = 4; y < i_num_lines - 4; y++ )
{
for( int x = 4; x < i_src_visible - 4; x++ )
{
uint32_t a =
abs((((int)p_smooth[(y - 1) * i_src_visible + x]
- (int)p_smooth[(y + 1) * i_src_visible + x]) * 2)
+ ((int)p_smooth[(y - 1) * i_src_visible + x - 1]
- (int)p_smooth[(y + 1) * i_src_visible + x - 1])
+ ((int)p_smooth[(y - 1) * i_src_visible + x + 1]
- (int)p_smooth[(y + 1) * i_src_visible + x + 1]))
+ abs((((int)p_smooth[y * i_src_visible + x - 1]
- (int)p_smooth[y * i_src_visible + x + 1]) * 2)
+ ((int)p_smooth[(y - 1) * i_src_visible + x - 1]
- (int)p_smooth[(y - 1) * i_src_visible + x + 1])
+ ((int)p_smooth[(y + 1) * i_src_visible + x - 1]
- (int)p_smooth[(y + 1) * i_src_visible + x + 1]));
if( a>>8 )
{
for( int i = 0; i < i_nb_steps; i++ )
{
i_rho = p_pre_hough[(i*i_num_lines+y)*i_src_visible + x];
if( p_hough[i_rho + i_diag/2 + i * i_diag]++ > i_max )
{
i_max = p_hough[i_rho + i_diag/2 + i * i_diag];
i_rho_max = i_rho;
i_phi_max = i;
}
}
}
}
}
d_sin = sin(i_phi_max*d_step);
d_cos = cos(i_phi_max*d_step);
if( d_cos != 0 )
{
for( int x = 0; x < i_src_visible; x++ )
{
int y = (i_rho_max - x * d_sin) / d_cos;
if( y >= 0 && y < i_num_lines )
p_outpix[y*i_dst_pitch+x] = 255;
}
}
free( p_hough );
free( p_smooth );
}
#undef p_pre_hough
static int GradientCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval,
void *p_data )
{
VLC_UNUSED(oldval);
filter_sys_t *p_sys = (filter_sys_t *)p_data;
vlc_mutex_lock( &p_sys->lock );
if( !strcmp( psz_var, FILTER_PREFIX "mode" ) )
{
if( !strcmp( newval.psz_string, "gradient" ) )
{
p_sys->i_mode = GRADIENT;
}
else if( !strcmp( newval.psz_string, "edge" ) )
{
p_sys->i_mode = EDGE;
}
else if( !strcmp( newval.psz_string, "hough" ) )
{
p_sys->i_mode = HOUGH;
}
else
{
msg_Err( p_this, "no valid gradient mode provided (%s)", newval.psz_string );
p_sys->i_mode = GRADIENT;
}
}
else if( !strcmp( psz_var, FILTER_PREFIX "type" ) )
{
p_sys->i_gradient_type = newval.i_int;
}
else if( !strcmp( psz_var, FILTER_PREFIX "cartoon" ) )
{
p_sys->b_cartoon = newval.b_bool;
}
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
