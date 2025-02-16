#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout.h>
#include <vlc_modules.h>
#include <vlc_picture.h>
#include <vlc_filter.h>
#include <vlc_image.h>
#include "filter_picture.h"
#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t* Filter( filter_t*, picture_t* );
static void ReleaseImages( filter_t* p_filter );
static void VlcPictureToIplImage( filter_t* p_filter, picture_t* p_in );
static const char *const chroma_list[] = { "input", "I420", "RGB32"};
static const char *const chroma_list_text[] = { N_("Use input chroma unaltered"),
N_("I420 - first plane is grayscale"), N_("RGB32")};
static const char *const output_list[] = { "none", "input", "processed"};
static const char *const output_list_text[] = { N_("Don't display any video"),
N_("Display the input video"), N_("Display the processed video")};
vlc_module_begin ()
set_description( N_("OpenCV video filter wrapper") )
set_shortname( N_("OpenCV" ))
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
set_capability( "video filter", 0 )
add_shortcut( "opencv_wrapper" )
set_callbacks( Create, Destroy )
add_float_with_range( "opencv-scale", 1.0, 0.1, 2.0,
N_("Scale factor (0.1-2.0)"),
N_("Amount by which to scale the picture before sending it to the internal OpenCV filter"),
false )
add_string( "opencv-chroma", "input",
N_("OpenCV filter chroma"),
N_("Chroma to convert picture to before sending it to the internal OpenCV filter"), false);
change_string_list( chroma_list, chroma_list_text )
add_string( "opencv-output", "input",
N_("Wrapper filter output"),
N_("Determines what (if any) video is displayed by the wrapper filter"), false);
change_string_list( output_list, output_list_text )
add_string( "opencv-filter-name", "none",
N_("OpenCV internal filter name"),
N_("Name of internal OpenCV plugin filter to use"), false);
vlc_module_end ()
enum wrapper_output_t
{
NONE,
VINPUT,
PROCESSED
};
enum internal_chroma_t
{
CINPUT,
GREY,
RGB
};
typedef struct
{
image_handler_t *p_image;
int i_cv_image_size;
picture_t *p_proc_image;
picture_t *p_to_be_freed;
float f_scale;
int i_wrapper_output;
int i_internal_chroma;
IplImage *p_cv_image[VOUT_MAX_PLANES];
filter_t *p_opencv;
char* psz_inner_name;
picture_t hacked_pic;
} filter_sys_t;
static int Create( vlc_object_t *p_this )
{
filter_t* p_filter = (filter_t*)p_this;
filter_sys_t *p_sys;
char *psz_chroma, *psz_output;
p_sys = malloc( sizeof( filter_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_sys->p_opencv = vlc_object_create( p_filter, sizeof(filter_t) );
if( !p_sys->p_opencv ) {
free( p_sys );
return VLC_ENOMEM;
}
p_sys->psz_inner_name = var_InheritString( p_filter, "opencv-filter-name" );
if( p_sys->psz_inner_name )
p_sys->p_opencv->p_module =
module_need( p_sys->p_opencv,
"opencv internal filter",
p_sys->psz_inner_name,
true );
if( !p_sys->p_opencv->p_module )
{
msg_Err( p_filter, "can't open internal opencv filter: %s", p_sys->psz_inner_name );
free( p_sys->psz_inner_name );
vlc_object_delete(p_sys->p_opencv);
free( p_sys );
return VLC_ENOMOD;
}
p_sys->p_image = image_HandlerCreate( p_filter );
for( int i = 0; i < VOUT_MAX_PLANES; i++ )
p_sys->p_cv_image[i] = NULL;
p_sys->p_proc_image = NULL;
p_sys->p_to_be_freed = NULL;
p_sys->i_cv_image_size = 0;
psz_chroma = var_InheritString( p_filter, "opencv-chroma" );
if( psz_chroma == NULL )
{
msg_Err( p_filter, "configuration variable %s empty, using 'grey'",
"opencv-chroma" );
p_sys->i_internal_chroma = GREY;
} else if( !strcmp( psz_chroma, "input" ) )
p_sys->i_internal_chroma = CINPUT;
else if( !strcmp( psz_chroma, "I420" ) )
p_sys->i_internal_chroma = GREY;
else if( !strcmp( psz_chroma, "RGB32" ) )
p_sys->i_internal_chroma = RGB;
else {
msg_Err( p_filter, "no valid opencv-chroma provided, using 'grey'" );
p_sys->i_internal_chroma = GREY;
}
free( psz_chroma );
psz_output = var_InheritString( p_filter, "opencv-output" );
if( psz_output == NULL )
{
msg_Err( p_filter, "configuration variable %s empty, using 'input'",
"opencv-output" );
p_sys->i_wrapper_output = VINPUT;
} else if( !strcmp( psz_output, "none" ) )
p_sys->i_wrapper_output = NONE;
else if( !strcmp( psz_output, "input" ) )
p_sys->i_wrapper_output = VINPUT;
else if( !strcmp( psz_output, "processed" ) )
p_sys->i_wrapper_output = PROCESSED;
else {
msg_Err( p_filter, "no valid opencv-output provided, using 'input'" );
p_sys->i_wrapper_output = VINPUT;
}
free( psz_output );
p_sys->f_scale =
var_InheritFloat( p_filter, "opencv-scale" );
msg_Info(p_filter, "Configuration: opencv-scale: %f, opencv-chroma: %d, "
"opencv-output: %d, opencv-filter %s",
p_sys->f_scale,
p_sys->i_internal_chroma,
p_sys->i_wrapper_output,
p_sys->psz_inner_name);
#if !defined(NDEBUG)
msg_Dbg( p_filter, "opencv_wrapper successfully started" );
#endif
p_filter->p_sys = p_sys;
p_filter->pf_video_filter = Filter;
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t* p_filter = (filter_t*)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
ReleaseImages( p_filter );
module_unneed( p_sys->p_opencv, p_sys->p_opencv->p_module );
vlc_object_delete(p_sys->p_opencv);
free( p_sys );
}
static void ReleaseImages( filter_t* p_filter )
{
filter_sys_t* p_sys = p_filter->p_sys;
for( int i = 0; i < VOUT_MAX_PLANES; i++ )
{
if (p_sys->p_cv_image[i] != NULL)
{
cvReleaseImageHeader(&(p_sys->p_cv_image[i]));
p_sys->p_cv_image[i] = NULL;
}
}
p_sys->i_cv_image_size = 0;
if (p_sys->p_to_be_freed)
{
picture_Release( p_sys->p_to_be_freed );
p_sys->p_to_be_freed = NULL;
}
#if !defined(NDEBUG)
msg_Dbg( p_filter, "images released" );
#endif
}
static void VlcPictureToIplImage( filter_t* p_filter, picture_t* p_in )
{
int planes = p_in->i_planes; 
CvSize sz = cvSize(p_in->format.i_width, p_in->format.i_height);
video_format_t fmt_out;
filter_sys_t* p_sys = p_filter->p_sys;
memset( &fmt_out, 0, sizeof(video_format_t) );
if ((p_sys->f_scale != 1) || (p_sys->i_internal_chroma != CINPUT))
{
fmt_out = p_in->format;
fmt_out.i_width = p_in->format.i_width * p_sys->f_scale;
fmt_out.i_height = p_in->format.i_height * p_sys->f_scale;
if (p_sys->i_internal_chroma == RGB)
{
fmt_out.i_chroma = VLC_CODEC_RGB24;
}
else if (p_sys->i_internal_chroma == GREY)
{
fmt_out.i_chroma = VLC_CODEC_I420;
}
p_sys->p_proc_image = image_Convert( p_sys->p_image, p_in,
&(p_in->format), &fmt_out );
if (!p_sys->p_proc_image)
{
msg_Err(p_filter, "can't convert (unsupported formats?), aborting...");
return;
}
}
else 
{
p_sys->p_proc_image = filter_NewPicture( p_filter ); 
picture_Copy( p_sys->p_proc_image, p_in );
}
p_sys->p_to_be_freed = p_sys->p_proc_image; 
planes = p_sys->p_proc_image->i_planes;
p_sys->i_cv_image_size = planes;
for( int i = 0; i < planes; i++ )
{
sz = cvSize(abs(p_sys->p_proc_image->p[i].i_visible_pitch /
p_sys->p_proc_image->p[i].i_pixel_pitch),
abs(p_sys->p_proc_image->p[i].i_visible_lines));
p_sys->p_cv_image[i] = cvCreateImageHeader(sz, IPL_DEPTH_8U,
p_sys->p_proc_image->p[i].i_pixel_pitch);
cvSetData( p_sys->p_cv_image[i],
(char*)(p_sys->p_proc_image->p[i].p_pixels), p_sys->p_proc_image->p[i].i_pitch );
}
p_sys->hacked_pic.i_planes = planes;
p_sys->hacked_pic.format.i_chroma = fmt_out.i_chroma;
#if !defined(NDEBUG)
msg_Dbg( p_filter, "VlcPictureToIplImageRgb() completed" );
#endif
}
static picture_t* Filter( filter_t* p_filter, picture_t* p_pic )
{
filter_sys_t *p_sys = p_filter->p_sys;
picture_t* p_outpic = filter_NewPicture( p_filter );
if( p_outpic == NULL ) {
msg_Err( p_filter, "couldn't get a p_outpic!" );
picture_Release( p_pic );
return NULL;
}
if (p_sys->i_wrapper_output == VINPUT)
picture_Copy( p_outpic, p_pic );
VlcPictureToIplImage( p_filter, p_pic );
p_sys->p_opencv->pf_video_filter( p_sys->p_opencv, (picture_t*)&(p_sys->p_cv_image[0]) );
if(p_sys->i_wrapper_output == PROCESSED) {
if( (p_sys->p_proc_image) &&
(p_sys->p_proc_image->i_planes > 0) &&
(p_sys->i_internal_chroma != CINPUT) ) {
video_format_t fmt_out = p_pic->format;
picture_t* p_outpic_tmp = image_Convert(
p_sys->p_image,
p_sys->p_proc_image,
&(p_sys->p_proc_image->format),
&fmt_out );
picture_CopyPixels( p_outpic, p_outpic_tmp );
CopyInfoAndRelease( p_outpic, p_outpic_tmp );
} else if( p_sys->i_internal_chroma == CINPUT ) {
picture_CopyPixels( p_outpic, p_sys->p_proc_image );
picture_CopyProperties( p_outpic, p_sys->p_proc_image );
}
}
ReleaseImages( p_filter );
picture_Release( p_pic );
#if !defined(NDEBUG)
msg_Dbg( p_filter, "Filter() done" );
#endif
if( p_sys->i_wrapper_output != NONE ) {
return p_outpic;
} else { 
picture_Release( p_outpic );
return NULL;
}
}
