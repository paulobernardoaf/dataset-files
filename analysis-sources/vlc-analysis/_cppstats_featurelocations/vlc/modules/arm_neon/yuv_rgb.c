




















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_cpu.h>
#include "arm_neon/chroma_neon.h"

static int Open (vlc_object_t *);

vlc_module_begin ()
set_description (N_("ARM NEON video chroma YUV->RGBA"))
set_capability ("video converter", 250)
set_callback(Open)
vlc_module_end ()





















































static void I420_RGBA (filter_t *filter, picture_t *src, picture_t *dst)
{
struct yuv_pack out = { dst->p->p_pixels, dst->p->i_pitch };
struct yuv_planes in = { src->Y_PIXELS, src->U_PIXELS, src->V_PIXELS, src->Y_PITCH };
i420_rgb_neon (&out, &in, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height);
}

static void I420_RV16 (filter_t *filter, picture_t *src, picture_t *dst)
{
struct yuv_pack out = { dst->p->p_pixels, dst->p->i_pitch };
struct yuv_planes in = { src->Y_PIXELS, src->U_PIXELS, src->V_PIXELS, src->Y_PITCH };
i420_rv16_neon (&out, &in, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height);
}

static void YV12_RGBA (filter_t *filter, picture_t *src, picture_t *dst)
{
struct yuv_pack out = { dst->p->p_pixels, dst->p->i_pitch };
struct yuv_planes in = { src->Y_PIXELS, src->V_PIXELS, src->U_PIXELS, src->Y_PITCH };
i420_rgb_neon (&out, &in, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height);
}

static void NV21_RGBA (filter_t *filter, picture_t *src, picture_t *dst)
{
struct yuv_pack out = { dst->p->p_pixels, dst->p->i_pitch };
struct yuv_planes in = { src->Y_PIXELS, src->U_PIXELS, src->V_PIXELS, src->Y_PITCH };
nv21_rgb_neon (&out, &in, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height);
}

static void NV12_RGBA (filter_t *filter, picture_t *src, picture_t *dst)
{
struct yuv_pack out = { dst->p->p_pixels, dst->p->i_pitch };
struct yuv_planes in = { src->Y_PIXELS, src->U_PIXELS, src->V_PIXELS, src->Y_PITCH };
nv12_rgb_neon (&out, &in, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height);
}

VIDEO_FILTER_WRAPPER (I420_RGBA)
VIDEO_FILTER_WRAPPER (I420_RV16)
VIDEO_FILTER_WRAPPER (YV12_RGBA)
VIDEO_FILTER_WRAPPER (NV21_RGBA)
VIDEO_FILTER_WRAPPER (NV12_RGBA)

static int Open (vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;

if (!vlc_CPU_ARM_NEON())
return VLC_EGENERIC;

if (((filter->fmt_in.video.i_width | filter->fmt_in.video.i_height) & 1)
|| (filter->fmt_in.video.i_width != filter->fmt_out.video.i_width)
|| (filter->fmt_in.video.i_height != filter->fmt_out.video.i_height)
|| (filter->fmt_in.video.orientation != filter->fmt_out.video.orientation))
return VLC_EGENERIC;

switch (filter->fmt_out.video.i_chroma)
{
case VLC_CODEC_RGB16:
switch (filter->fmt_in.video.i_chroma)
{
case VLC_CODEC_I420:
filter->pf_video_filter = I420_RV16_Filter;
break;
default:
return VLC_EGENERIC;
}
break;

case VLC_CODEC_RGB32:
if( filter->fmt_out.video.i_rmask != 0x000000ff
|| filter->fmt_out.video.i_gmask != 0x0000ff00
|| filter->fmt_out.video.i_bmask != 0x00ff0000 )
return VLC_EGENERIC;

switch (filter->fmt_in.video.i_chroma)
{
case VLC_CODEC_I420:
filter->pf_video_filter = I420_RGBA_Filter;
break;
case VLC_CODEC_YV12:
filter->pf_video_filter = YV12_RGBA_Filter;
break;
case VLC_CODEC_NV21:
filter->pf_video_filter = NV21_RGBA_Filter;
break;
case VLC_CODEC_NV12:
filter->pf_video_filter = NV12_RGBA_Filter;
break;
default:
return VLC_EGENERIC;
}
break;

default:
return VLC_EGENERIC;
}
















msg_Dbg(filter, "%4.4s(%dx%d) to %4.4s(%dx%d)",
(char*)&filter->fmt_in.video.i_chroma, filter->fmt_in.video.i_visible_width, filter->fmt_in.video.i_visible_height,
(char*)&filter->fmt_out.video.i_chroma, filter->fmt_out.video.i_visible_width, filter->fmt_out.video.i_visible_height);

return VLC_SUCCESS;
}
