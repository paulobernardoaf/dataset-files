






















#include "libavutil/internal.h"
#include "avfilter.h"
#include "internal.h"
#include "video.h"

static const AVFilterPad avfilter_vf_null_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

static const AVFilterPad avfilter_vf_null_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

AVFilter ff_vf_null = {
.name = "null",
.description = NULL_IF_CONFIG_SMALL("Pass the source unchanged to the output."),
.inputs = avfilter_vf_null_inputs,
.outputs = avfilter_vf_null_outputs,
};
