

















#include "avfilter.h"
#include "internal.h"
#include "libavutil/internal.h"

static int filter_frame(AVFilterLink *link, AVFrame *frame)
{
av_frame_free(&frame);
return 0;
}

static const AVFilterPad avfilter_vsink_nullsink_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL },
};

AVFilter ff_vsink_nullsink = {
.name = "nullsink",
.description = NULL_IF_CONFIG_SMALL("Do absolutely nothing with the input video."),
.priv_size = 0,
.inputs = avfilter_vsink_nullsink_inputs,
.outputs = NULL,
};
