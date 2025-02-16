#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_image.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_url.h>
#include "filter_picture.h"
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t *Filter( filter_t *, picture_t * );
static void FilterErase( filter_t *, picture_t *, picture_t * );
static int EraseCallback( vlc_object_t *, char const *,
vlc_value_t, vlc_value_t, void * );
#define MASK_TEXT N_("Image mask")
#define MASK_LONGTEXT N_("Image mask. Pixels with an alpha value greater than 50% will be erased.")
#define POSX_TEXT N_("X coordinate")
#define POSX_LONGTEXT N_("X coordinate of the mask.")
#define POSY_TEXT N_("Y coordinate")
#define POSY_LONGTEXT N_("Y coordinate of the mask.")
#define ERASE_HELP N_("Remove zones of the video using a picture as mask")
#define CFG_PREFIX "erase-"
vlc_module_begin ()
set_description( N_("Erase video filter") )
set_shortname( N_( "Erase" ))
set_capability( "video filter", 0 )
set_help(ERASE_HELP)
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
add_loadfile(CFG_PREFIX "mask", NULL, MASK_TEXT, MASK_LONGTEXT)
add_integer( CFG_PREFIX "x", 0, POSX_TEXT, POSX_LONGTEXT, false )
add_integer( CFG_PREFIX "y", 0, POSY_TEXT, POSY_LONGTEXT, false )
add_shortcut( "erase" )
set_callbacks( Create, Destroy )
vlc_module_end ()
static const char *const ppsz_filter_options[] = {
"mask", "x", "y", NULL
};
typedef struct
{
int i_x;
int i_y;
picture_t *p_mask;
vlc_mutex_t lock;
} filter_sys_t;
static void LoadMask( filter_t *p_filter, const char *psz_filename )
{
filter_sys_t *p_sys = p_filter->p_sys;
image_handler_t *p_image;
video_format_t fmt_out;
picture_t *p_old_mask = p_sys->p_mask;
video_format_Init( &fmt_out, VLC_CODEC_YUVA );
p_image = image_HandlerCreate( p_filter );
char *psz_url = vlc_path2uri( psz_filename, NULL );
p_sys->p_mask = image_ReadUrl( p_image, psz_url, &fmt_out );
free( psz_url );
video_format_Clean( &fmt_out );
if( p_sys->p_mask )
{
if( p_old_mask )
picture_Release( p_old_mask );
}
else if( p_old_mask )
{
p_sys->p_mask = p_old_mask;
msg_Err( p_filter, "Error while loading new mask. Keeping old mask." );
}
else
msg_Err( p_filter, "Error while loading new mask. No mask available." );
image_HandlerDelete( p_image );
}
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys;
char *psz_filename;
switch( p_filter->fmt_in.video.i_chroma )
{
case VLC_CODEC_I420:
case VLC_CODEC_J420:
case VLC_CODEC_YV12:
case VLC_CODEC_I422:
case VLC_CODEC_J422:
break;
default:
msg_Err( p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_filter->fmt_in.video.i_chroma) );
return VLC_EGENERIC;
}
p_filter->p_sys = malloc( sizeof( filter_sys_t ) );
if( p_filter->p_sys == NULL )
return VLC_ENOMEM;
p_sys = p_filter->p_sys;
p_filter->pf_video_filter = Filter;
config_ChainParse( p_filter, CFG_PREFIX, ppsz_filter_options,
p_filter->p_cfg );
psz_filename =
var_CreateGetNonEmptyStringCommand( p_filter, CFG_PREFIX "mask" );
if( !psz_filename )
{
msg_Err( p_filter, "Missing 'mask' option value." );
free( p_sys );
return VLC_EGENERIC;
}
p_sys->p_mask = NULL;
LoadMask( p_filter, psz_filename );
free( psz_filename );
p_sys->i_x = var_CreateGetIntegerCommand( p_filter, CFG_PREFIX "x" );
p_sys->i_y = var_CreateGetIntegerCommand( p_filter, CFG_PREFIX "y" );
vlc_mutex_init( &p_sys->lock );
var_AddCallback( p_filter, CFG_PREFIX "x", EraseCallback, p_sys );
var_AddCallback( p_filter, CFG_PREFIX "y", EraseCallback, p_sys );
var_AddCallback( p_filter, CFG_PREFIX "mask", EraseCallback, p_sys );
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
if( p_sys->p_mask )
picture_Release( p_sys->p_mask );
var_DelCallback( p_filter, CFG_PREFIX "x", EraseCallback, p_sys );
var_DelCallback( p_filter, CFG_PREFIX "y", EraseCallback, p_sys );
var_DelCallback( p_filter, CFG_PREFIX "mask", EraseCallback, p_sys );
free( p_filter->p_sys );
}
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;
filter_sys_t *p_sys = p_filter->p_sys;
if( !p_pic ) return NULL;
p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
picture_Release( p_pic );
return NULL;
}
vlc_mutex_lock( &p_sys->lock );
if( p_sys->p_mask )
FilterErase( p_filter, p_pic, p_outpic );
else
picture_CopyPixels( p_outpic, p_pic );
vlc_mutex_unlock( &p_sys->lock );
return CopyInfoAndRelease( p_outpic, p_pic );
}
static void FilterErase( filter_t *p_filter, picture_t *p_inpic,
picture_t *p_outpic )
{
filter_sys_t *p_sys = p_filter->p_sys;
const int i_mask_pitch = p_sys->p_mask->A_PITCH;
const int i_mask_visible_pitch = p_sys->p_mask->p[A_PLANE].i_visible_pitch;
const int i_mask_visible_lines = p_sys->p_mask->p[A_PLANE].i_visible_lines;
for( int i_plane = 0; i_plane < p_inpic->i_planes; i_plane++ )
{
const int i_pitch = p_outpic->p[i_plane].i_pitch;
const int i_2pitch = i_pitch<<1;
const int i_visible_pitch = p_inpic->p[i_plane].i_visible_pitch;
const int i_visible_lines = p_inpic->p[i_plane].i_visible_lines;
uint8_t *p_mask = p_sys->p_mask->A_PIXELS;
int i_x = p_sys->i_x, i_y = p_sys->i_y;
int i_height = i_mask_visible_lines;
int i_width = i_mask_visible_pitch;
const bool b_line_factor = ( i_plane &&
!( p_inpic->format.i_chroma == VLC_CODEC_I422
|| p_inpic->format.i_chroma == VLC_CODEC_J422 ) );
if( i_plane ) 
{
i_width >>= 1;
i_x >>= 1;
}
if( b_line_factor )
{
i_height >>= 1;
i_y >>= 1;
}
i_height = __MIN( i_visible_lines - i_y, i_height );
i_width = __MIN( i_visible_pitch - i_x, i_width );
plane_CopyPixels( &p_outpic->p[i_plane], &p_inpic->p[i_plane] );
uint8_t *p_outpix = p_outpic->p[i_plane].p_pixels + i_y*i_pitch + i_x;
for( int y = 0; y < i_height;
y++, p_mask += i_mask_pitch, p_outpix += i_pitch )
{
uint8_t prev, next = 0;
int prev_x = -1, next_x = -2;
int quot = 0;
if( i_x )
{
prev = *(p_outpix-1);
}
else if( y || i_y )
{
prev = *(p_outpix-i_pitch);
}
else
{
prev = 0xff;
}
for( int x = 0; x < i_width; x++ )
{
if( p_mask[i_plane?x<<1:x] > 127 )
{
if( next_x <= prev_x )
{
int x0;
for( x0 = x; x0 < i_width; x0++ )
{
if( p_mask[i_plane?x0<<1:x0] <= 127 )
{
next_x = x0;
next = p_outpix[x0];
break;
}
}
if( next_x <= prev_x )
{
if( x0 == x ) x0++;
if( x0 < i_visible_pitch )
{
next_x = x0;
next = p_outpix[x0];
}
else
{
next_x = x0;
next = prev;
}
}
if( !( i_x || y || i_y ) )
prev = next;
quot = ((next-prev)<<16)/(next_x-prev_x);
}
p_outpix[x] = prev + (((x-prev_x)*quot+(1<<16))>>16);
}
else
{
prev = p_outpix[x];
prev_x = x;
}
}
}
p_mask = p_sys->p_mask->A_PIXELS;
i_height = b_line_factor ? i_mask_visible_lines>>1
: i_mask_visible_lines;
i_height = __MIN( i_visible_lines - i_y - 2, i_height );
int y = __MAX(i_y,2);
p_outpix = p_outpic->p[i_plane].p_pixels + (i_y+y)*i_pitch + i_x;
for( ; y < i_height; y++, p_mask += i_mask_pitch, p_outpix += i_pitch )
{
for( int x = 0; x < i_width; x++ )
{
if( p_mask[i_plane?x<<1:x] > 127 )
{
p_outpix[x] =
( (p_outpix[x-i_2pitch]<<1) 
+ (p_outpix[x-i_pitch ]<<2) 
+ (p_outpix[x ]<<2) 
+ (p_outpix[x+i_pitch ]<<2) 
+ (p_outpix[x+i_2pitch]<<1) )>>4; 
}
}
}
}
}
static int EraseCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval, void *p_data )
{
VLC_UNUSED(oldval);
filter_sys_t *p_sys = (filter_sys_t *)p_data;
if( !strcmp( psz_var, CFG_PREFIX "x" ) )
{
vlc_mutex_lock( &p_sys->lock );
p_sys->i_x = newval.i_int;
vlc_mutex_unlock( &p_sys->lock );
}
else if( !strcmp( psz_var, CFG_PREFIX "y" ) )
{
vlc_mutex_lock( &p_sys->lock );
p_sys->i_y = newval.i_int;
vlc_mutex_unlock( &p_sys->lock );
}
else if( !strcmp( psz_var, CFG_PREFIX "mask" ) )
{
vlc_mutex_lock( &p_sys->lock );
LoadMask( (filter_t*)p_this, newval.psz_string );
vlc_mutex_unlock( &p_sys->lock );
}
else
{
msg_Warn( p_this, "Unknown callback command." );
}
return VLC_SUCCESS;
}
