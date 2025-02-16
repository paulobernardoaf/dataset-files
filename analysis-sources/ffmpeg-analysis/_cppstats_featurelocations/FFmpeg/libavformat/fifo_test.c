




















#include <stdlib.h>

#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/avassert.h"

#include "avformat.h"
#include "url.h"



#define MAX_TST_PACKETS 128
#define SLEEPTIME_50_MS 50000
#define SLEEPTIME_10_MS 10000





typedef struct FailingMuxerPacketData {
int ret; 
int recover_after; 
unsigned sleep_time; 
} FailingMuxerPacketData;


typedef struct FailingMuxerContext {
AVClass *class;
int write_header_ret;
int write_trailer_ret;

int print_deinit_summary;

int flush_count;
int pts_written[MAX_TST_PACKETS];
int pts_written_nr;
} FailingMuxerContext;

static int failing_write_header(AVFormatContext *avf)
{
FailingMuxerContext *ctx = avf->priv_data;
return ctx->write_header_ret;
}

static int failing_write_packet(AVFormatContext *avf, AVPacket *pkt)
{
FailingMuxerContext *ctx = avf->priv_data;
int ret = 0;
if (!pkt) {
ctx->flush_count++;
} else {
FailingMuxerPacketData *data = (FailingMuxerPacketData*) pkt->data;

if (!data->recover_after) {
data->ret = 0;
} else {
data->recover_after--;
}

ret = data->ret;

if (data->sleep_time) {
int64_t slept = 0;
while (slept < data->sleep_time) {
if (ff_check_interrupt(&avf->interrupt_callback))
return AVERROR_EXIT;
av_usleep(SLEEPTIME_10_MS);
slept += SLEEPTIME_10_MS;
}
}

if (!ret) {
ctx->pts_written[ctx->pts_written_nr++] = pkt->pts;
av_packet_unref(pkt);
}
}
return ret;
}

static int failing_write_trailer(AVFormatContext *avf)
{
FailingMuxerContext *ctx = avf->priv_data;
return ctx->write_trailer_ret;
}

static void failing_deinit(AVFormatContext *avf)
{
int i;
FailingMuxerContext *ctx = avf->priv_data;

if (!ctx->print_deinit_summary)
return;

printf("flush count: %d\n", ctx->flush_count);
printf("pts seen nr: %d\n", ctx->pts_written_nr);
printf("pts seen: ");
for (i = 0; i < ctx->pts_written_nr; ++i ) {
printf(i ? ",%d" : "%d", ctx->pts_written[i]);
}
printf("\n");
}
#define OFFSET(x) offsetof(FailingMuxerContext, x)
static const AVOption options[] = {
{"write_header_ret", "write_header() return value", OFFSET(write_header_ret),
AV_OPT_TYPE_INT, {.i64 = 0}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM},
{"write_trailer_ret", "write_trailer() return value", OFFSET(write_trailer_ret),
AV_OPT_TYPE_INT, {.i64 = 0}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM},
{"print_deinit_summary", "print summary when deinitializing muxer", OFFSET(print_deinit_summary),
AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, AV_OPT_FLAG_ENCODING_PARAM},
{NULL}
};

static const AVClass failing_muxer_class = {
.class_name = "Fifo test muxer",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

AVOutputFormat ff_fifo_test_muxer = {
.name = "fifo_test",
.long_name = NULL_IF_CONFIG_SMALL("Fifo test muxer"),
.priv_data_size = sizeof(FailingMuxerContext),
.write_header = failing_write_header,
.write_packet = failing_write_packet,
.write_trailer = failing_write_trailer,
.deinit = failing_deinit,
.priv_class = &failing_muxer_class,
.flags = AVFMT_NOFILE | AVFMT_ALLOW_FLUSH,
};

