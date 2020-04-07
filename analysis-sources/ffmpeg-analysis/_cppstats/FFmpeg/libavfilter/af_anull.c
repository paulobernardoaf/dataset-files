#include "audio.h"
#include "avfilter.h"
#include "internal.h"
#include "libavutil/internal.h"
static const AVFilterPad avfilter_af_anull_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
static const AVFilterPad avfilter_af_anull_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
AVFilter ff_af_anull = {
.name = "anull",
.description = NULL_IF_CONFIG_SMALL("Pass the source unchanged to the output."),
.inputs = avfilter_af_anull_inputs,
.outputs = avfilter_af_anull_outputs,
};
