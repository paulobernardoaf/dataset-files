

















#include "audio.h"
#include "avfilter.h"
#include "internal.h"

static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
AVFilterLink *outlink = inlink->dst->outputs[0];
AVFrame *out = ff_get_audio_buffer(outlink, in->nb_samples);
int ret;

if (!out) {
ret = AVERROR(ENOMEM);
goto fail;
}

ret = av_frame_copy_props(out, in);
if (ret < 0)
goto fail;
ret = av_frame_copy(out, in);
if (ret < 0)
goto fail;
av_frame_free(&in);
return ff_filter_frame(outlink, out);
fail:
av_frame_free(&in);
av_frame_free(&out);
return ret;
}

static const AVFilterPad acopy_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad acopy_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};

AVFilter ff_af_acopy = {
.name = "acopy",
.description = NULL_IF_CONFIG_SMALL("Copy the input audio unchanged to the output."),
.inputs = acopy_inputs,
.outputs = acopy_outputs,
};
