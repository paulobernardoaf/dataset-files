



















#include "libavutil/internal.h"
#include "avfilter.h"
#include "internal.h"

static int null_filter_frame(AVFilterLink *link, AVFrame *frame)
{
av_frame_free(&frame);
return 0;
}

static const AVFilterPad avfilter_asink_anullsink_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = null_filter_frame,
},
{ NULL },
};

AVFilter ff_asink_anullsink = {
.name = "anullsink",
.description = NULL_IF_CONFIG_SMALL("Do absolutely nothing with the input audio."),
.priv_size = 0,
.inputs = avfilter_asink_anullsink_inputs,
.outputs = NULL,
};
