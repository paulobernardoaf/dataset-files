#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <stdatomic.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
#define SIG_TEXT N_("Sharpen strength (0-2)")
#define SIG_LONGTEXT N_("Set the Sharpen strength, between 0 and 2. Defaults to 0.05.")
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t *Filter( filter_t *, picture_t * );
static int SharpenCallback( vlc_object_t *, char const *,
vlc_value_t, vlc_value_t, void * );
#define SHARPEN_HELP N_("Augment contrast between contours.")
#define FILTER_PREFIX "sharpen-"
vlc_module_begin ()
set_description( N_("Sharpen video filter") )
set_shortname( N_("Sharpen") )
set_help(SHARPEN_HELP)
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
set_capability( "video filter", 0 )
add_float_with_range( FILTER_PREFIX "sigma", 0.05, 0.0, 2.0,
SIG_TEXT, SIG_LONGTEXT, false )
change_safe()
add_shortcut( "sharpen" )
set_callbacks( Create, Destroy )
vlc_module_end ()
static const char *const ppsz_filter_options[] = {
"sigma", NULL
};
typedef struct
{
atomic_int sigma;
} filter_sys_t;
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
const vlc_fourcc_t fourcc = p_filter->fmt_in.video.i_chroma;
const vlc_chroma_description_t *p_chroma = vlc_fourcc_GetChromaDescription( fourcc );
if( !p_chroma || p_chroma->plane_count != 3 ||
(p_chroma->pixel_size != 1 &&
p_filter->fmt_in.video.i_chroma != VLC_CODEC_I420_10L &&
p_filter->fmt_in.video.i_chroma != VLC_CODEC_I420_10B)) {
msg_Dbg( p_filter, "Unsupported chroma (%4.4s)", (char*)&fourcc );
return VLC_EGENERIC;
}
filter_sys_t *p_sys = malloc( sizeof( filter_sys_t ) );
if( p_sys == NULL )
return VLC_ENOMEM;
p_filter->p_sys = p_sys;
p_filter->pf_video_filter = Filter;
config_ChainParse( p_filter, FILTER_PREFIX, ppsz_filter_options,
p_filter->p_cfg );
atomic_init(&p_sys->sigma,
var_CreateGetFloatCommand(p_filter, FILTER_PREFIX "sigma")
* (1 << 20));
var_AddCallback( p_filter, FILTER_PREFIX "sigma",
SharpenCallback, p_sys );
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
var_DelCallback( p_filter, FILTER_PREFIX "sigma", SharpenCallback, p_sys );
free( p_sys );
}
#define IS_YUV_420_10BITS(fmt) (fmt == VLC_CODEC_I420_10L || fmt == VLC_CODEC_I420_10B)
#define SHARPEN_FRAME(maxval, data_t) do { assert((maxval) >= 0); data_t *restrict p_src = (data_t *)p_pic->p[Y_PLANE].p_pixels; data_t *restrict p_out = (data_t *)p_outpic->p[Y_PLANE].p_pixels; const unsigned data_sz = sizeof(data_t); const int i_src_line_len = p_pic->p[Y_PLANE].i_pitch / data_sz; const int i_out_line_len = p_outpic->p[Y_PLANE].i_pitch / data_sz; const int sigma = atomic_load(&p_sys->sigma); memcpy(p_out, p_src, i_visible_pitch); for( unsigned i = 1; i < i_visible_lines - 1; i++ ) { p_out[i * i_out_line_len] = p_src[i * i_src_line_len]; for( unsigned j = data_sz; j < i_visible_pitch - 1; j++ ) { const int line_idx_1 = (i - 1) * i_src_line_len; const int line_idx_2 = i * i_src_line_len; const int line_idx_3 = (i + 1) * i_src_line_len; int pix = (p_src[line_idx_1 + j - 1] * v1) + (p_src[line_idx_1 + j ] * v1) + (p_src[line_idx_1 + j + 1] * v1) + (p_src[line_idx_2 + j - 1] * v1) + (p_src[line_idx_2 + j ] << v2) + (p_src[line_idx_2 + j + 1] * v1) + (p_src[line_idx_3 + j - 1] * v1) + (p_src[line_idx_3 + j ] * v1) + (p_src[line_idx_3 + j + 1] * v1); pix = (VLC_CLIP(pix, -(maxval), maxval) * sigma) >> 20; p_out[i * i_out_line_len + j] = VLC_CLIP( p_src[line_idx_2 + j] + pix, 0, maxval); } p_out[i * i_out_line_len + i_visible_pitch / data_sz - 1] = p_src[i * i_src_line_len + i_visible_pitch / data_sz - 1]; } memcpy(&p_out[(i_visible_lines - 1) * i_out_line_len], &p_src[(i_visible_lines - 1) * i_src_line_len], i_visible_pitch); } while (0)
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;
const int v1 = -1;
const int v2 = 3; 
const unsigned i_visible_lines = p_pic->p[Y_PLANE].i_visible_lines;
const unsigned i_visible_pitch = p_pic->p[Y_PLANE].i_visible_pitch;
p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
picture_Release( p_pic );
return NULL;
}
filter_sys_t *p_sys = p_filter->p_sys;
if (!IS_YUV_420_10BITS(p_pic->format.i_chroma))
SHARPEN_FRAME(255, uint8_t);
else
SHARPEN_FRAME(1023, uint16_t);
plane_CopyPixels( &p_outpic->p[U_PLANE], &p_pic->p[U_PLANE] );
plane_CopyPixels( &p_outpic->p[V_PLANE], &p_pic->p[V_PLANE] );
return CopyInfoAndRelease( p_outpic, p_pic );
}
static int SharpenCallback( vlc_object_t *p_this, char const *psz_var,
vlc_value_t oldval, vlc_value_t newval,
void *p_data )
{
VLC_UNUSED(p_this); VLC_UNUSED(oldval); VLC_UNUSED(psz_var);
filter_sys_t *p_sys = (filter_sys_t *)p_data;
atomic_store(&p_sys->sigma,
VLC_CLIP(newval.f_float, 0.f, 2.f) * (1 << 20));
return VLC_SUCCESS;
}
