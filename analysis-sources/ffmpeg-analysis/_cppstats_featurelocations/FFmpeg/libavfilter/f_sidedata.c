






















#include "libavutil/avassert.h"
#include "libavutil/internal.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "formats.h"
#include "internal.h"

enum SideDataMode {
SIDEDATA_SELECT,
SIDEDATA_DELETE,
SIDEDATA_NB
};

typedef struct SideDataContext {
const AVClass *class;

int mode;
int type; 
} SideDataContext;

#define OFFSET(x) offsetof(SideDataContext, x)
#if FF_API_FRAME_QP
#define DEFINE_OPTIONS(filt_name, FLAGS) static const AVOption filt_name##_options[] = { { "mode", "set a mode of operation", OFFSET(mode), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, SIDEDATA_NB-1, FLAGS, "mode" }, { "select", "select frame", 0, AV_OPT_TYPE_CONST, {.i64 = SIDEDATA_SELECT }, 0, 0, FLAGS, "mode" }, { "delete", "delete side data", 0, AV_OPT_TYPE_CONST, {.i64 = SIDEDATA_DELETE }, 0, 0, FLAGS, "mode" }, { "type", "set side data type", OFFSET(type), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, INT_MAX, FLAGS, "type" }, { "PANSCAN", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_PANSCAN }, 0, 0, FLAGS, "type" }, { "A53_CC", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_A53_CC }, 0, 0, FLAGS, "type" }, { "STEREO3D", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_STEREO3D }, 0, 0, FLAGS, "type" }, { "MATRIXENCODING", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MATRIXENCODING }, 0, 0, FLAGS, "type" }, { "DOWNMIX_INFO", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DOWNMIX_INFO }, 0, 0, FLAGS, "type" }, { "REPLAYGAIN", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_REPLAYGAIN }, 0, 0, FLAGS, "type" }, { "DISPLAYMATRIX", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DISPLAYMATRIX }, 0, 0, FLAGS, "type" }, { "AFD", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_AFD }, 0, 0, FLAGS, "type" }, { "MOTION_VECTORS", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MOTION_VECTORS }, 0, 0, FLAGS, "type" }, { "SKIP_SAMPLES", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_SKIP_SAMPLES }, 0, 0, FLAGS, "type" }, { "AUDIO_SERVICE_TYPE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_AUDIO_SERVICE_TYPE }, 0, 0, FLAGS, "type" }, { "MASTERING_DISPLAY_METADATA", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MASTERING_DISPLAY_METADATA }, 0, 0, FLAGS, "type" }, { "GOP_TIMECODE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_GOP_TIMECODE }, 0, 0, FLAGS, "type" }, { "SPHERICAL", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_SPHERICAL }, 0, 0, FLAGS, "type" }, { "CONTENT_LIGHT_LEVEL", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_CONTENT_LIGHT_LEVEL }, 0, 0, FLAGS, "type" }, { "ICC_PROFILE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_ICC_PROFILE }, 0, 0, FLAGS, "type" }, { "QP_TABLE_PROPERTIES", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_QP_TABLE_PROPERTIES }, 0, 0, FLAGS, "type" }, { "QP_TABLE_DATA", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_QP_TABLE_DATA }, 0, 0, FLAGS, "type" }, { "S12M_TIMECOD", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_S12M_TIMECODE }, 0, 0, FLAGS, "type" }, { "DYNAMIC_HDR_PLUS", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DYNAMIC_HDR_PLUS }, 0, 0, FLAGS, "type" }, { "REGIONS_OF_INTEREST", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_REGIONS_OF_INTEREST }, 0, 0, FLAGS, "type" }, { NULL } }




























#else
#define DEFINE_OPTIONS(filt_name, FLAGS) static const AVOption filt_name##_options[] = { { "mode", "set a mode of operation", OFFSET(mode), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, SIDEDATA_NB-1, FLAGS, "mode" }, { "select", "select frame", 0, AV_OPT_TYPE_CONST, {.i64 = SIDEDATA_SELECT }, 0, 0, FLAGS, "mode" }, { "delete", "delete side data", 0, AV_OPT_TYPE_CONST, {.i64 = SIDEDATA_DELETE }, 0, 0, FLAGS, "mode" }, { "type", "set side data type", OFFSET(type), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, INT_MAX, FLAGS, "type" }, { "PANSCAN", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_PANSCAN }, 0, 0, FLAGS, "type" }, { "A53_CC", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_A53_CC }, 0, 0, FLAGS, "type" }, { "STEREO3D", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_STEREO3D }, 0, 0, FLAGS, "type" }, { "MATRIXENCODING", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MATRIXENCODING }, 0, 0, FLAGS, "type" }, { "DOWNMIX_INFO", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DOWNMIX_INFO }, 0, 0, FLAGS, "type" }, { "REPLAYGAIN", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_REPLAYGAIN }, 0, 0, FLAGS, "type" }, { "DISPLAYMATRIX", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DISPLAYMATRIX }, 0, 0, FLAGS, "type" }, { "AFD", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_AFD }, 0, 0, FLAGS, "type" }, { "MOTION_VECTORS", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MOTION_VECTORS }, 0, 0, FLAGS, "type" }, { "SKIP_SAMPLES", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_SKIP_SAMPLES }, 0, 0, FLAGS, "type" }, { "AUDIO_SERVICE_TYPE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_AUDIO_SERVICE_TYPE }, 0, 0, FLAGS, "type" }, { "MASTERING_DISPLAY_METADATA", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_MASTERING_DISPLAY_METADATA }, 0, 0, FLAGS, "type" }, { "GOP_TIMECODE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_GOP_TIMECODE }, 0, 0, FLAGS, "type" }, { "SPHERICAL", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_SPHERICAL }, 0, 0, FLAGS, "type" }, { "CONTENT_LIGHT_LEVEL", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_CONTENT_LIGHT_LEVEL }, 0, 0, FLAGS, "type" }, { "ICC_PROFILE", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_ICC_PROFILE }, 0, 0, FLAGS, "type" }, { "S12M_TIMECOD", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_S12M_TIMECODE }, 0, 0, FLAGS, "type" }, { "DYNAMIC_HDR_PLUS", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_DYNAMIC_HDR_PLUS }, 0, 0, FLAGS, "type" }, { "REGIONS_OF_INTEREST", "", 0, AV_OPT_TYPE_CONST, {.i64 = AV_FRAME_DATA_REGIONS_OF_INTEREST }, 0, 0, FLAGS, "type" }, { NULL } }


























#endif

static av_cold int init(AVFilterContext *ctx)
{
SideDataContext *s = ctx->priv;

if (s->type == -1 && s->mode != SIDEDATA_DELETE) {
av_log(ctx, AV_LOG_ERROR, "Side data type must be set\n");
return AVERROR(EINVAL);
}

return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
AVFilterContext *ctx = inlink->dst;
AVFilterLink *outlink = ctx->outputs[0];
SideDataContext *s = ctx->priv;
AVFrameSideData *sd = NULL;

if (s->type != -1)
sd = av_frame_get_side_data(frame, s->type);

switch (s->mode) {
case SIDEDATA_SELECT:
if (sd) {
return ff_filter_frame(outlink, frame);
}
break;
case SIDEDATA_DELETE:
if (s->type == -1) {
while (frame->nb_side_data)
av_frame_remove_side_data(frame, frame->side_data[0]->type);
} else if (sd) {
av_frame_remove_side_data(frame, s->type);
}
return ff_filter_frame(outlink, frame);
break;
default:
av_assert0(0);
};

av_frame_free(&frame);

return 0;
}

#if CONFIG_ASIDEDATA_FILTER

DEFINE_OPTIONS(asidedata, AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_FILTERING_PARAM);
AVFILTER_DEFINE_CLASS(asidedata);

static const AVFilterPad ainputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad aoutputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};

AVFilter ff_af_asidedata = {
.name = "asidedata",
.description = NULL_IF_CONFIG_SMALL("Manipulate audio frame side data."),
.priv_size = sizeof(SideDataContext),
.priv_class = &asidedata_class,
.init = init,
.inputs = ainputs,
.outputs = aoutputs,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
#endif 

#if CONFIG_SIDEDATA_FILTER

DEFINE_OPTIONS(sidedata, AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM);
AVFILTER_DEFINE_CLASS(sidedata);

static const AVFilterPad inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

AVFilter ff_vf_sidedata = {
.name = "sidedata",
.description = NULL_IF_CONFIG_SMALL("Manipulate video frame side data."),
.priv_size = sizeof(SideDataContext),
.priv_class = &sidedata_class,
.init = init,
.inputs = inputs,
.outputs = outputs,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
#endif 
