



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdlib.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "vlc_vdpau.h"

typedef struct
{
vlc_tick_t last_pts;
} filter_sys_t;

static picture_t *Deinterlace(filter_t *filter, picture_t *src)
{
filter_sys_t *sys = filter->p_sys;
vlc_tick_t last_pts = sys->last_pts;

sys->last_pts = src->date;

vlc_vdp_video_field_t *f1 = VDPAU_FIELD_FROM_PICCTX(src->context);
if (unlikely(f1 == NULL))
return src;
if (f1->structure != VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME)
return src; 

#if defined(VOUT_CORE_GETS_A_CLUE)
picture_t *dst = filter_NewPicture(filter);
#else
picture_t *dst = picture_NewFromFormat(&src->format);
#endif
if (dst == NULL)
return src; 

vlc_vdp_video_field_t *f2 = vlc_vdp_video_copy(f1); 
if (unlikely(f2 == NULL))
{
picture_Release(dst);
return src;
}

picture_CopyProperties(dst, src);
dst->context = &f2->context;

if (last_pts != VLC_TICK_INVALID)
dst->date = (3 * src->date - last_pts) / 2;
else
if (filter->fmt_in.video.i_frame_rate != 0)
dst->date = src->date + vlc_tick_from_samples(filter->fmt_in.video.i_frame_rate_base
,filter->fmt_in.video.i_frame_rate);
dst->b_top_field_first = !src->b_top_field_first;
dst->i_nb_fields = 1;
src->i_nb_fields = 1;

assert(src->p_next == NULL);
src->p_next = dst;

if (src->b_progressive || src->b_top_field_first)
{
f1->structure = VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD;
f2->structure = VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD;
}
else
{
f1->structure = VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD;
f2->structure = VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD;
}

src->b_progressive = true;
dst->b_progressive = true;
return src;
}

static int Open(vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;

if ( filter->vctx_in == NULL ||
vlc_video_context_GetType(filter->vctx_in) != VLC_VIDEO_CONTEXT_VDPAU )
return VLC_EGENERIC;
if (filter->fmt_in.video.i_chroma != VLC_CODEC_VDPAU_VIDEO_420
&& filter->fmt_in.video.i_chroma != VLC_CODEC_VDPAU_VIDEO_422
&& filter->fmt_in.video.i_chroma != VLC_CODEC_VDPAU_VIDEO_444)
return VLC_EGENERIC;
if (!video_format_IsSimilar(&filter->fmt_in.video, &filter->fmt_out.video))
return VLC_EGENERIC;

filter_sys_t *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;




sys->last_pts = VLC_TICK_INVALID;

filter->pf_video_filter = Deinterlace;
filter->p_sys = sys;
filter->fmt_out.video.i_frame_rate *= 2;
filter->vctx_out = vlc_video_context_Hold(filter->vctx_in);
return VLC_SUCCESS;
}

static void Close(vlc_object_t *obj)
{
filter_t *filter = (filter_t *)obj;
filter_sys_t *sys = filter->p_sys;

vlc_video_context_Release(filter->vctx_out);
free(sys);
}

vlc_module_begin()
set_description(N_("VDPAU deinterlacing filter"))
set_capability("video filter", 0)
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VFILTER)
set_callbacks(Open, Close)
add_shortcut ("deinterlace")
vlc_module_end()
