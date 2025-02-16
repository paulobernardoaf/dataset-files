



























#include <float.h>
#include <stdint.h>

#include "libavutil/avassert.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "filters.h"
#include "internal.h"

enum EOFAction {
EOF_ACTION_ROUND,
EOF_ACTION_PASS,
EOF_ACTION_NB
};

typedef struct FPSContext {
const AVClass *class;

double start_time; 

AVRational framerate; 
int rounding; 
int eof_action; 


int64_t in_pts_off; 
int64_t out_pts_off; 


int status; 
int64_t status_pts; 

AVFrame *frames[2]; 
int frames_count; 

int64_t next_pts; 


int cur_frame_out; 
int frames_in; 
int frames_out; 
int dup; 
int drop; 
} FPSContext;

#define OFFSET(x) offsetof(FPSContext, x)
#define V AV_OPT_FLAG_VIDEO_PARAM
#define F AV_OPT_FLAG_FILTERING_PARAM
static const AVOption fps_options[] = {
{ "fps", "A string describing desired output framerate", OFFSET(framerate), AV_OPT_TYPE_VIDEO_RATE, { .str = "25" }, 0, INT_MAX, V|F },
{ "start_time", "Assume the first PTS should be this value.", OFFSET(start_time), AV_OPT_TYPE_DOUBLE, { .dbl = DBL_MAX}, -DBL_MAX, DBL_MAX, V|F },
{ "round", "set rounding method for timestamps", OFFSET(rounding), AV_OPT_TYPE_INT, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
{ "zero", "round towards 0", 0, AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_ZERO }, 0, 0, V|F, "round" },
{ "inf", "round away from 0", 0, AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_INF }, 0, 0, V|F, "round" },
{ "down", "round towards -infty", 0, AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_DOWN }, 0, 0, V|F, "round" },
{ "up", "round towards +infty", 0, AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_UP }, 0, 0, V|F, "round" },
{ "near", "round to nearest", 0, AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_NEAR_INF }, 0, 0, V|F, "round" },
{ "eof_action", "action performed for last frame", OFFSET(eof_action), AV_OPT_TYPE_INT, { .i64 = EOF_ACTION_ROUND }, 0, EOF_ACTION_NB-1, V|F, "eof_action" },
{ "round", "round similar to other frames", 0, AV_OPT_TYPE_CONST, { .i64 = EOF_ACTION_ROUND }, 0, 0, V|F, "eof_action" },
{ "pass", "pass through last frame", 0, AV_OPT_TYPE_CONST, { .i64 = EOF_ACTION_PASS }, 0, 0, V|F, "eof_action" },
{ NULL }
};

AVFILTER_DEFINE_CLASS(fps);

static av_cold int init(AVFilterContext *ctx)
{
FPSContext *s = ctx->priv;

s->status_pts = AV_NOPTS_VALUE;
s->next_pts = AV_NOPTS_VALUE;

av_log(ctx, AV_LOG_VERBOSE, "fps=%d/%d\n", s->framerate.num, s->framerate.den);
return 0;
}


static AVFrame *shift_frame(AVFilterContext *ctx, FPSContext *s)
{
AVFrame *frame;


av_assert1(s->frames_count > 0);

frame = s->frames[0];
s->frames[0] = s->frames[1];
s->frames[1] = NULL;
s->frames_count--;


s->frames_out += s->cur_frame_out;
if (s->cur_frame_out > 1) {
av_log(ctx, AV_LOG_DEBUG, "Duplicated frame with pts %"PRId64" %d times\n",
frame->pts, s->cur_frame_out - 1);
s->dup += s->cur_frame_out - 1;
} else if (s->cur_frame_out == 0) {
av_log(ctx, AV_LOG_DEBUG, "Dropping frame with pts %"PRId64"\n",
frame->pts);
s->drop++;
}
s->cur_frame_out = 0;

return frame;
}

static av_cold void uninit(AVFilterContext *ctx)
{
FPSContext *s = ctx->priv;

AVFrame *frame;

while (s->frames_count > 0) {
frame = shift_frame(ctx, s);
av_frame_free(&frame);
}

av_log(ctx, AV_LOG_VERBOSE, "%d frames in, %d frames out; %d frames dropped, "
"%d frames duplicated.\n", s->frames_in, s->frames_out, s->drop, s->dup);
}

static int config_props(AVFilterLink* outlink)
{
AVFilterContext *ctx = outlink->src;
AVFilterLink *inlink = ctx->inputs[0];
FPSContext *s = ctx->priv;

outlink->time_base = av_inv_q(s->framerate);
outlink->frame_rate = s->framerate;


if (s->start_time != DBL_MAX && s->start_time != AV_NOPTS_VALUE) {
double first_pts = s->start_time * AV_TIME_BASE;
if (first_pts < INT64_MIN || first_pts > INT64_MAX) {
av_log(ctx, AV_LOG_ERROR, "Start time %f cannot be represented in internal time base\n",
s->start_time);
return AVERROR(EINVAL);
}
s->in_pts_off = av_rescale_q_rnd(first_pts, AV_TIME_BASE_Q, inlink->time_base,
s->rounding | AV_ROUND_PASS_MINMAX);
s->out_pts_off = av_rescale_q_rnd(first_pts, AV_TIME_BASE_Q, outlink->time_base,
s->rounding | AV_ROUND_PASS_MINMAX);
s->next_pts = s->out_pts_off;
av_log(ctx, AV_LOG_VERBOSE, "Set first pts to (in:%"PRId64" out:%"PRId64") from start time %f\n",
s->in_pts_off, s->out_pts_off, s->start_time);
}

return 0;
}


static int read_frame(AVFilterContext *ctx, FPSContext *s, AVFilterLink *inlink, AVFilterLink *outlink)
{
AVFrame *frame;
int ret;
int64_t in_pts;


av_assert1(s->frames_count < 2);

ret = ff_inlink_consume_frame(inlink, &frame);

av_assert1(ret);
if (ret < 0)
return ret;




in_pts = frame->pts;
frame->pts = s->out_pts_off + av_rescale_q_rnd(in_pts - s->in_pts_off,
inlink->time_base, outlink->time_base,
s->rounding | AV_ROUND_PASS_MINMAX);

av_log(ctx, AV_LOG_DEBUG, "Read frame with in pts %"PRId64", out pts %"PRId64"\n",
in_pts, frame->pts);

s->frames[s->frames_count++] = frame;
s->frames_in++;

return 1;
}


static int write_frame(AVFilterContext *ctx, FPSContext *s, AVFilterLink *outlink, int *again)
{
AVFrame *frame;

av_assert1(s->frames_count == 2 || (s->status && s->frames_count == 1));


if (s->next_pts == AV_NOPTS_VALUE) {
if (s->frames[0]->pts != AV_NOPTS_VALUE) {
s->next_pts = s->frames[0]->pts;
av_log(ctx, AV_LOG_VERBOSE, "Set first pts to %"PRId64"\n", s->next_pts);
} else {
av_log(ctx, AV_LOG_WARNING, "Discarding initial frame(s) with no "
"timestamp.\n");
frame = shift_frame(ctx, s);
av_frame_free(&frame);
*again = 1;
return 0;
}
}






if ((s->frames_count == 2 && s->frames[1]->pts <= s->next_pts) ||
(s->status && s->status_pts <= s->next_pts)) {

frame = shift_frame(ctx, s);
av_frame_free(&frame);
*again = 1;
return 0;


} else {
frame = av_frame_clone(s->frames[0]);
if (!frame)
return AVERROR(ENOMEM);

av_frame_remove_side_data(s->frames[0], AV_FRAME_DATA_A53_CC);
frame->pts = s->next_pts++;

av_log(ctx, AV_LOG_DEBUG, "Writing frame with pts %"PRId64" to pts %"PRId64"\n",
s->frames[0]->pts, frame->pts);
s->cur_frame_out++;
*again = 1;
return ff_filter_frame(outlink, frame);
}
}


static void update_eof_pts(AVFilterContext *ctx, FPSContext *s, AVFilterLink *inlink, AVFilterLink *outlink, int64_t status_pts)
{
int eof_rounding = (s->eof_action == EOF_ACTION_PASS) ? AV_ROUND_UP : s->rounding;
s->status_pts = av_rescale_q_rnd(status_pts, inlink->time_base, outlink->time_base,
eof_rounding | AV_ROUND_PASS_MINMAX);

av_log(ctx, AV_LOG_DEBUG, "EOF is at pts %"PRId64"\n", s->status_pts);
}

static int activate(AVFilterContext *ctx)
{
FPSContext *s = ctx->priv;
AVFilterLink *inlink = ctx->inputs[0];
AVFilterLink *outlink = ctx->outputs[0];

int ret;
int again = 0;
int64_t status_pts;

FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);


if (!s->status) {


while (s->frames_count < 2 && ff_inlink_check_available_frame(inlink)) {
ret = read_frame(ctx, s, inlink, outlink);
if (ret < 0)
return ret;
}


if (s->frames_count < 2) {

ret = ff_inlink_acknowledge_status(inlink, &s->status, &status_pts);
if (ret > 0)
update_eof_pts(ctx, s, inlink, outlink, status_pts);

if (!ret) {

FF_FILTER_FORWARD_WANTED(outlink, inlink);
return 0;
}
}
}


if (s->frames_count > 0) {
ret = write_frame(ctx, s, outlink, &again);

if (again)
ff_filter_set_ready(ctx, 100);
return ret;
}


if (s->status && s->frames_count == 0) {
ff_outlink_set_status(outlink, s->status, s->next_pts);
return 0;
}

return FFERROR_NOT_READY;
}

static const AVFilterPad avfilter_vf_fps_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

static const AVFilterPad avfilter_vf_fps_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_props,
},
{ NULL }
};

AVFilter ff_vf_fps = {
.name = "fps",
.description = NULL_IF_CONFIG_SMALL("Force constant framerate."),
.init = init,
.uninit = uninit,
.priv_size = sizeof(FPSContext),
.priv_class = &fps_class,
.activate = activate,
.inputs = avfilter_vf_fps_inputs,
.outputs = avfilter_vf_fps_outputs,
};
