





















#include <string.h>
#include <stdio.h>

#include "libavutil/avassert.h"
#include "libavutil/buffer.h"
#include "libavutil/hwcontext.h"
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"

#include "avfilter.h"
#include "internal.h"
#include "video.h"

#define BUFFER_ALIGN 32


AVFrame *ff_null_get_video_buffer(AVFilterLink *link, int w, int h)
{
return ff_get_video_buffer(link->dst->outputs[0], w, h);
}

AVFrame *ff_default_get_video_buffer(AVFilterLink *link, int w, int h)
{
AVFrame *frame = NULL;
int pool_width = 0;
int pool_height = 0;
int pool_align = 0;
enum AVPixelFormat pool_format = AV_PIX_FMT_NONE;

if (link->hw_frames_ctx &&
((AVHWFramesContext*)link->hw_frames_ctx->data)->format == link->format) {
int ret;
AVFrame *frame = av_frame_alloc();

if (!frame)
return NULL;

ret = av_hwframe_get_buffer(link->hw_frames_ctx, frame, 0);
if (ret < 0)
av_frame_free(&frame);

return frame;
}

if (!link->frame_pool) {
link->frame_pool = ff_frame_pool_video_init(av_buffer_allocz, w, h,
link->format, BUFFER_ALIGN);
if (!link->frame_pool)
return NULL;
} else {
if (ff_frame_pool_get_video_config(link->frame_pool,
&pool_width, &pool_height,
&pool_format, &pool_align) < 0) {
return NULL;
}

if (pool_width != w || pool_height != h ||
pool_format != link->format || pool_align != BUFFER_ALIGN) {

ff_frame_pool_uninit((FFFramePool **)&link->frame_pool);
link->frame_pool = ff_frame_pool_video_init(av_buffer_allocz, w, h,
link->format, BUFFER_ALIGN);
if (!link->frame_pool)
return NULL;
}
}

frame = ff_frame_pool_get(link->frame_pool);
if (!frame)
return NULL;

frame->sample_aspect_ratio = link->sample_aspect_ratio;

return frame;
}

AVFrame *ff_get_video_buffer(AVFilterLink *link, int w, int h)
{
AVFrame *ret = NULL;

FF_TPRINTF_START(NULL, get_video_buffer); ff_tlog_link(NULL, link, 0);

if (link->dstpad->get_video_buffer)
ret = link->dstpad->get_video_buffer(link, w, h);

if (!ret)
ret = ff_default_get_video_buffer(link, w, h);

return ret;
}
