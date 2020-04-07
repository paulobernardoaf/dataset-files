#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
static int Create ( vlc_object_t * );
static void Destroy ( vlc_object_t * );
static picture_t *Filter( filter_t *, picture_t * );
vlc_module_begin ()
set_description( N_("Invert video filter") )
set_shortname( N_("Color inversion" ))
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
set_capability( "video filter", 0 )
add_shortcut( "invert" )
set_callbacks( Create, Destroy )
vlc_module_end ()
static int Create( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *)p_this;
vlc_fourcc_t fourcc = p_filter->fmt_in.video.i_chroma;
if( fourcc == VLC_CODEC_YUVP || fourcc == VLC_CODEC_RGBP
|| fourcc == VLC_CODEC_RGBA || fourcc == VLC_CODEC_ARGB
|| fourcc == VLC_CODEC_BGRA )
return VLC_EGENERIC;
const vlc_chroma_description_t *p_chroma =
vlc_fourcc_GetChromaDescription( fourcc );
if( p_chroma == NULL || p_chroma->plane_count == 0
|| p_chroma->pixel_size * 8 != p_chroma->pixel_bits )
return VLC_EGENERIC;
p_filter->pf_video_filter = Filter;
return VLC_SUCCESS;
}
static void Destroy( vlc_object_t *p_this )
{
(void)p_this;
}
static picture_t *Filter( filter_t *p_filter, picture_t *p_pic )
{
picture_t *p_outpic;
int i_planes;
if( !p_pic ) return NULL;
p_outpic = filter_NewPicture( p_filter );
if( !p_outpic )
{
msg_Warn( p_filter, "can't get output picture" );
picture_Release( p_pic );
return NULL;
}
if( p_pic->format.i_chroma == VLC_CODEC_YUVA )
{
i_planes = p_pic->i_planes - 1;
memcpy(
p_outpic->p[A_PLANE].p_pixels, p_pic->p[A_PLANE].p_pixels,
p_pic->p[A_PLANE].i_pitch * p_pic->p[A_PLANE].i_lines );
}
else
{
i_planes = p_pic->i_planes;
}
for( int i_index = 0 ; i_index < i_planes ; i_index++ )
{
uint8_t *p_in, *p_in_end, *p_line_end, *p_out;
p_in = p_pic->p[i_index].p_pixels;
p_in_end = p_in + p_pic->p[i_index].i_visible_lines
* p_pic->p[i_index].i_pitch;
p_out = p_outpic->p[i_index].p_pixels;
while( p_in < p_in_end )
{
uint64_t *p_in64, *p_out64;
p_line_end = p_in + p_pic->p[i_index].i_visible_pitch - 64;
p_in64 = (uint64_t*)p_in;
p_out64 = (uint64_t*)p_out;
while( p_in64 < (uint64_t *)p_line_end )
{
*p_out64++ = ~*p_in64++; *p_out64++ = ~*p_in64++;
*p_out64++ = ~*p_in64++; *p_out64++ = ~*p_in64++;
*p_out64++ = ~*p_in64++; *p_out64++ = ~*p_in64++;
*p_out64++ = ~*p_in64++; *p_out64++ = ~*p_in64++;
}
p_in = (uint8_t*)p_in64;
p_out = (uint8_t*)p_out64;
p_line_end += 64;
while( p_in < p_line_end )
{
*p_out++ = ~( *p_in++ );
}
p_in += p_pic->p[i_index].i_pitch
- p_pic->p[i_index].i_visible_pitch;
p_out += p_outpic->p[i_index].i_pitch
- p_outpic->p[i_index].i_visible_pitch;
}
}
return CopyInfoAndRelease( p_outpic, p_pic );
}
