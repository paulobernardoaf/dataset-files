







































#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "libavutil/channel_layout.h"
#include "libavutil/md5.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#define INPUT_SAMPLERATE 48000
#define INPUT_FORMAT AV_SAMPLE_FMT_FLTP
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_5POINT0

#define VOLUME_VAL 0.90

static int init_filter_graph(AVFilterGraph **graph, AVFilterContext **src,
AVFilterContext **sink)
{
AVFilterGraph *filter_graph;
AVFilterContext *abuffer_ctx;
const AVFilter *abuffer;
AVFilterContext *volume_ctx;
const AVFilter *volume;
AVFilterContext *aformat_ctx;
const AVFilter *aformat;
AVFilterContext *abuffersink_ctx;
const AVFilter *abuffersink;

AVDictionary *options_dict = NULL;
uint8_t options_str[1024];
uint8_t ch_layout[64];

int err;


filter_graph = avfilter_graph_alloc();
if (!filter_graph) {
fprintf(stderr, "Unable to create filter graph.\n");
return AVERROR(ENOMEM);
}



abuffer = avfilter_get_by_name("abuffer");
if (!abuffer) {
fprintf(stderr, "Could not find the abuffer filter.\n");
return AVERROR_FILTER_NOT_FOUND;
}

abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src");
if (!abuffer_ctx) {
fprintf(stderr, "Could not allocate the abuffer instance.\n");
return AVERROR(ENOMEM);
}


av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, INPUT_CHANNEL_LAYOUT);
av_opt_set (abuffer_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
av_opt_set (abuffer_ctx, "sample_fmt", av_get_sample_fmt_name(INPUT_FORMAT), AV_OPT_SEARCH_CHILDREN);
av_opt_set_q (abuffer_ctx, "time_base", (AVRational){ 1, INPUT_SAMPLERATE }, AV_OPT_SEARCH_CHILDREN);
av_opt_set_int(abuffer_ctx, "sample_rate", INPUT_SAMPLERATE, AV_OPT_SEARCH_CHILDREN);



err = avfilter_init_str(abuffer_ctx, NULL);
if (err < 0) {
fprintf(stderr, "Could not initialize the abuffer filter.\n");
return err;
}


volume = avfilter_get_by_name("volume");
if (!volume) {
fprintf(stderr, "Could not find the volume filter.\n");
return AVERROR_FILTER_NOT_FOUND;
}

volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "volume");
if (!volume_ctx) {
fprintf(stderr, "Could not allocate the volume instance.\n");
return AVERROR(ENOMEM);
}



av_dict_set(&options_dict, "volume", AV_STRINGIFY(VOLUME_VAL), 0);
err = avfilter_init_dict(volume_ctx, &options_dict);
av_dict_free(&options_dict);
if (err < 0) {
fprintf(stderr, "Could not initialize the volume filter.\n");
return err;
}



aformat = avfilter_get_by_name("aformat");
if (!aformat) {
fprintf(stderr, "Could not find the aformat filter.\n");
return AVERROR_FILTER_NOT_FOUND;
}

aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
if (!aformat_ctx) {
fprintf(stderr, "Could not allocate the aformat instance.\n");
return AVERROR(ENOMEM);
}



snprintf(options_str, sizeof(options_str),
"sample_fmts=%s:sample_rates=%d:channel_layouts=0x%"PRIx64,
av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), 44100,
(uint64_t)AV_CH_LAYOUT_STEREO);
err = avfilter_init_str(aformat_ctx, options_str);
if (err < 0) {
av_log(NULL, AV_LOG_ERROR, "Could not initialize the aformat filter.\n");
return err;
}



abuffersink = avfilter_get_by_name("abuffersink");
if (!abuffersink) {
fprintf(stderr, "Could not find the abuffersink filter.\n");
return AVERROR_FILTER_NOT_FOUND;
}

abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
if (!abuffersink_ctx) {
fprintf(stderr, "Could not allocate the abuffersink instance.\n");
return AVERROR(ENOMEM);
}


err = avfilter_init_str(abuffersink_ctx, NULL);
if (err < 0) {
fprintf(stderr, "Could not initialize the abuffersink instance.\n");
return err;
}



err = avfilter_link(abuffer_ctx, 0, volume_ctx, 0);
if (err >= 0)
err = avfilter_link(volume_ctx, 0, aformat_ctx, 0);
if (err >= 0)
err = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
if (err < 0) {
fprintf(stderr, "Error connecting filters\n");
return err;
}


err = avfilter_graph_config(filter_graph, NULL);
if (err < 0) {
av_log(NULL, AV_LOG_ERROR, "Error configuring the filter graph\n");
return err;
}

*graph = filter_graph;
*src = abuffer_ctx;
*sink = abuffersink_ctx;

return 0;
}



static int process_output(struct AVMD5 *md5, AVFrame *frame)
{
int planar = av_sample_fmt_is_planar(frame->format);
int channels = av_get_channel_layout_nb_channels(frame->channel_layout);
int planes = planar ? channels : 1;
int bps = av_get_bytes_per_sample(frame->format);
int plane_size = bps * frame->nb_samples * (planar ? 1 : channels);
int i, j;

for (i = 0; i < planes; i++) {
uint8_t checksum[16];

av_md5_init(md5);
av_md5_sum(checksum, frame->extended_data[i], plane_size);

fprintf(stdout, "plane %d: 0x", i);
for (j = 0; j < sizeof(checksum); j++)
fprintf(stdout, "%02X", checksum[j]);
fprintf(stdout, "\n");
}
fprintf(stdout, "\n");

return 0;
}



static int get_input(AVFrame *frame, int frame_num)
{
int err, i, j;

#define FRAME_SIZE 1024


frame->sample_rate = INPUT_SAMPLERATE;
frame->format = INPUT_FORMAT;
frame->channel_layout = INPUT_CHANNEL_LAYOUT;
frame->nb_samples = FRAME_SIZE;
frame->pts = frame_num * FRAME_SIZE;

err = av_frame_get_buffer(frame, 0);
if (err < 0)
return err;


for (i = 0; i < 5; i++) {
float *data = (float*)frame->extended_data[i];

for (j = 0; j < frame->nb_samples; j++)
data[j] = sin(2 * M_PI * (frame_num + j) * (i + 1) / FRAME_SIZE);
}

return 0;
}

int main(int argc, char *argv[])
{
struct AVMD5 *md5;
AVFilterGraph *graph;
AVFilterContext *src, *sink;
AVFrame *frame;
uint8_t errstr[1024];
float duration;
int err, nb_frames, i;

if (argc < 2) {
fprintf(stderr, "Usage: %s <duration>\n", argv[0]);
return 1;
}

duration = atof(argv[1]);
nb_frames = duration * INPUT_SAMPLERATE / FRAME_SIZE;
if (nb_frames <= 0) {
fprintf(stderr, "Invalid duration: %s\n", argv[1]);
return 1;
}


frame = av_frame_alloc();
if (!frame) {
fprintf(stderr, "Error allocating the frame\n");
return 1;
}

md5 = av_md5_alloc();
if (!md5) {
fprintf(stderr, "Error allocating the MD5 context\n");
return 1;
}


err = init_filter_graph(&graph, &src, &sink);
if (err < 0) {
fprintf(stderr, "Unable to init filter graph:");
goto fail;
}


for (i = 0; i < nb_frames; i++) {

err = get_input(frame, i);
if (err < 0) {
fprintf(stderr, "Error generating input frame:");
goto fail;
}


err = av_buffersrc_add_frame(src, frame);
if (err < 0) {
av_frame_unref(frame);
fprintf(stderr, "Error submitting the frame to the filtergraph:");
goto fail;
}


while ((err = av_buffersink_get_frame(sink, frame)) >= 0) {

err = process_output(md5, frame);
if (err < 0) {
fprintf(stderr, "Error processing the filtered frame:");
goto fail;
}
av_frame_unref(frame);
}

if (err == AVERROR(EAGAIN)) {

continue;
} else if (err == AVERROR_EOF) {

break;
} else if (err < 0) {

fprintf(stderr, "Error filtering the data:");
goto fail;
}
}

avfilter_graph_free(&graph);
av_frame_free(&frame);
av_freep(&md5);

return 0;

fail:
av_strerror(err, errstr, sizeof(errstr));
fprintf(stderr, "%s\n", errstr);
return 1;
}
