#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avfilter.h"
#include "internal.h"
#include "video.h"
typedef struct FlipContext {
const AVClass *class;
int vsub; 
} FlipContext;
static const AVOption vflip_options[] = {
{ NULL }
};
AVFILTER_DEFINE_CLASS(vflip);
static int config_input(AVFilterLink *link)
{
FlipContext *flip = link->dst->priv;
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(link->format);
flip->vsub = desc->log2_chroma_h;
return 0;
}
static AVFrame *get_video_buffer(AVFilterLink *link, int w, int h)
{
FlipContext *flip = link->dst->priv;
AVFrame *frame;
int i;
frame = ff_get_video_buffer(link->dst->outputs[0], w, h);
if (!frame)
return NULL;
for (i = 0; i < 4; i ++) {
int vsub = i == 1 || i == 2 ? flip->vsub : 0;
int height = AV_CEIL_RSHIFT(h, vsub);
if (frame->data[i]) {
frame->data[i] += (height - 1) * frame->linesize[i];
frame->linesize[i] = -frame->linesize[i];
}
}
return frame;
}
static int filter_frame(AVFilterLink *link, AVFrame *frame)
{
FlipContext *flip = link->dst->priv;
int i;
for (i = 0; i < 4; i ++) {
int vsub = i == 1 || i == 2 ? flip->vsub : 0;
int height = AV_CEIL_RSHIFT(link->h, vsub);
if (frame->data[i]) {
frame->data[i] += (height - 1) * frame->linesize[i];
frame->linesize[i] = -frame->linesize[i];
}
}
return ff_filter_frame(link->dst->outputs[0], frame);
}
static const AVFilterPad avfilter_vf_vflip_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.get_video_buffer = get_video_buffer,
.filter_frame = filter_frame,
.config_props = config_input,
},
{ NULL }
};
static const AVFilterPad avfilter_vf_vflip_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
AVFilter ff_vf_vflip = {
.name = "vflip",
.description = NULL_IF_CONFIG_SMALL("Flip the input video vertically."),
.priv_size = sizeof(FlipContext),
.priv_class = &vflip_class,
.inputs = avfilter_vf_vflip_inputs,
.outputs = avfilter_vf_vflip_outputs,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
