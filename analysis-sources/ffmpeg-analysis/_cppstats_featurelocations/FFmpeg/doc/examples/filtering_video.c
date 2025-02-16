




























#define _XOPEN_SOURCE 600 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

const char *filter_descr = "scale=78:24,transpose=cclock";




static AVFormatContext *fmt_ctx;
static AVCodecContext *dec_ctx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int video_stream_index = -1;
static int64_t last_pts = AV_NOPTS_VALUE;

static int open_input_file(const char *filename)
{
int ret;
AVCodec *dec;

if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
return ret;
}

if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
return ret;
}


ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
return ret;
}
video_stream_index = ret;


dec_ctx = avcodec_alloc_context3(dec);
if (!dec_ctx)
return AVERROR(ENOMEM);
avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);


if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
return ret;
}

return 0;
}

static int init_filters(const char *filters_descr)
{
char args[512];
int ret = 0;
const AVFilter *buffersrc = avfilter_get_by_name("buffer");
const AVFilter *buffersink = avfilter_get_by_name("buffersink");
AVFilterInOut *outputs = avfilter_inout_alloc();
AVFilterInOut *inputs = avfilter_inout_alloc();
AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

filter_graph = avfilter_graph_alloc();
if (!outputs || !inputs || !filter_graph) {
ret = AVERROR(ENOMEM);
goto end;
}


snprintf(args, sizeof(args),
"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
time_base.num, time_base.den,
dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);

ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
args, NULL, filter_graph);
if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
goto end;
}


ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
NULL, NULL, filter_graph);
if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
goto end;
}

ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
goto end;
}












outputs->name = av_strdup("in");
outputs->filter_ctx = buffersrc_ctx;
outputs->pad_idx = 0;
outputs->next = NULL;







inputs->name = av_strdup("out");
inputs->filter_ctx = buffersink_ctx;
inputs->pad_idx = 0;
inputs->next = NULL;

if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
&inputs, &outputs, NULL)) < 0)
goto end;

if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
goto end;

end:
avfilter_inout_free(&inputs);
avfilter_inout_free(&outputs);

return ret;
}

static void display_frame(const AVFrame *frame, AVRational time_base)
{
int x, y;
uint8_t *p0, *p;
int64_t delay;

if (frame->pts != AV_NOPTS_VALUE) {
if (last_pts != AV_NOPTS_VALUE) {


delay = av_rescale_q(frame->pts - last_pts,
time_base, AV_TIME_BASE_Q);
if (delay > 0 && delay < 1000000)
usleep(delay);
}
last_pts = frame->pts;
}


p0 = frame->data[0];
puts("\033c");
for (y = 0; y < frame->height; y++) {
p = p0;
for (x = 0; x < frame->width; x++)
putchar(" .-+#"[*(p++) / 52]);
putchar('\n');
p0 += frame->linesize[0];
}
fflush(stdout);
}

int main(int argc, char **argv)
{
int ret;
AVPacket packet;
AVFrame *frame;
AVFrame *filt_frame;

if (argc != 2) {
fprintf(stderr, "Usage: %s file\n", argv[0]);
exit(1);
}

frame = av_frame_alloc();
filt_frame = av_frame_alloc();
if (!frame || !filt_frame) {
perror("Could not allocate frame");
exit(1);
}

if ((ret = open_input_file(argv[1])) < 0)
goto end;
if ((ret = init_filters(filter_descr)) < 0)
goto end;


while (1) {
if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
break;

if (packet.stream_index == video_stream_index) {
ret = avcodec_send_packet(dec_ctx, &packet);
if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
break;
}

while (ret >= 0) {
ret = avcodec_receive_frame(dec_ctx, frame);
if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
break;
} else if (ret < 0) {
av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
goto end;
}

frame->pts = frame->best_effort_timestamp;


if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
break;
}


while (1) {
ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
break;
if (ret < 0)
goto end;
display_frame(filt_frame, buffersink_ctx->inputs[0]->time_base);
av_frame_unref(filt_frame);
}
av_frame_unref(frame);
}
}
av_packet_unref(&packet);
}
end:
avfilter_graph_free(&filter_graph);
avcodec_free_context(&dec_ctx);
avformat_close_input(&fmt_ctx);
av_frame_free(&frame);
av_frame_free(&filt_frame);

if (ret < 0 && ret != AVERROR_EOF) {
fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
exit(1);
}

exit(0);
}
