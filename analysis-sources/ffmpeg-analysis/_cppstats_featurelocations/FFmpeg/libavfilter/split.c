
























#include <stdio.h>

#include "libavutil/attributes.h"
#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"

#include "avfilter.h"
#include "audio.h"
#include "filters.h"
#include "formats.h"
#include "internal.h"
#include "video.h"

typedef struct SplitContext {
const AVClass *class;
int nb_outputs;
} SplitContext;

static av_cold int split_init(AVFilterContext *ctx)
{
SplitContext *s = ctx->priv;
int i, ret;

for (i = 0; i < s->nb_outputs; i++) {
AVFilterPad pad = { 0 };

pad.type = ctx->filter->inputs[0].type;
pad.name = av_asprintf("output%d", i);
if (!pad.name)
return AVERROR(ENOMEM);

if ((ret = ff_insert_outpad(ctx, i, &pad)) < 0) {
av_freep(&pad.name);
return ret;
}
}

return 0;
}

static av_cold void split_uninit(AVFilterContext *ctx)
{
int i;

for (i = 0; i < ctx->nb_outputs; i++)
av_freep(&ctx->output_pads[i].name);
}

static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
AVFilterContext *ctx = inlink->dst;
int i, ret = AVERROR_EOF;

for (i = 0; i < ctx->nb_outputs; i++) {
AVFrame *buf_out;

if (ff_outlink_get_status(ctx->outputs[i]))
continue;
buf_out = av_frame_clone(frame);
if (!buf_out) {
ret = AVERROR(ENOMEM);
break;
}

ret = ff_filter_frame(ctx->outputs[i], buf_out);
if (ret < 0)
break;
}
av_frame_free(&frame);
return ret;
}

#define OFFSET(x) offsetof(SplitContext, x)
#define FLAGS (AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_FILTERING_PARAM)
static const AVOption options[] = {
{ "outputs", "set number of outputs", OFFSET(nb_outputs), AV_OPT_TYPE_INT, { .i64 = 2 }, 1, INT_MAX, FLAGS },
{ NULL }
};

#define split_options options
AVFILTER_DEFINE_CLASS(split);

#define asplit_options options
AVFILTER_DEFINE_CLASS(asplit);

static const AVFilterPad avfilter_vf_split_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};

AVFilter ff_vf_split = {
.name = "split",
.description = NULL_IF_CONFIG_SMALL("Pass on the input to N video outputs."),
.priv_size = sizeof(SplitContext),
.priv_class = &split_class,
.init = split_init,
.uninit = split_uninit,
.inputs = avfilter_vf_split_inputs,
.outputs = NULL,
.flags = AVFILTER_FLAG_DYNAMIC_OUTPUTS,
};

static const AVFilterPad avfilter_af_asplit_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
},
{ NULL }
};

AVFilter ff_af_asplit = {
.name = "asplit",
.description = NULL_IF_CONFIG_SMALL("Pass on the audio input to N audio outputs."),
.priv_size = sizeof(SplitContext),
.priv_class = &asplit_class,
.init = split_init,
.uninit = split_uninit,
.inputs = avfilter_af_asplit_inputs,
.outputs = NULL,
.flags = AVFILTER_FLAG_DYNAMIC_OUTPUTS,
};
