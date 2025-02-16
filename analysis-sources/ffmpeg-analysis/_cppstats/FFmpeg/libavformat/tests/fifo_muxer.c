#include <stdlib.h>
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavformat/network.h"
#define MAX_TST_PACKETS 128
#define SLEEPTIME_50_MS 50000
#define SLEEPTIME_10_MS 10000
typedef struct FailingMuxerPacketData {
int ret; 
int recover_after; 
unsigned sleep_time; 
} FailingMuxerPacketData;
static int prepare_packet(AVPacket *pkt, const FailingMuxerPacketData *pkt_data, int64_t pts)
{
int ret;
FailingMuxerPacketData *data = av_malloc(sizeof(*data));
if (!data) {
return AVERROR(ENOMEM);
}
memcpy(data, pkt_data, sizeof(FailingMuxerPacketData));
ret = av_packet_from_data(pkt, (uint8_t*) data, sizeof(*data));
pkt->pts = pkt->dts = pts;
pkt->duration = 1;
return ret;
}
static int initialize_fifo_tst_muxer_chain(AVFormatContext **oc)
{
int ret = 0;
AVStream *s;
ret = avformat_alloc_output_context2(oc, NULL, "fifo", "-");
if (ret) {
fprintf(stderr, "Failed to create format context: %s\n",
av_err2str(ret));
return EXIT_FAILURE;
}
s = avformat_new_stream(*oc, NULL);
if (!s) {
fprintf(stderr, "Failed to create stream: %s\n",
av_err2str(ret));
ret = AVERROR(ENOMEM);
}
return ret;
}
static int fifo_basic_test(AVFormatContext *oc, AVDictionary **opts,
const FailingMuxerPacketData *pkt_data)
{
int ret = 0, i;
AVPacket pkt;
av_init_packet(&pkt);
ret = avformat_write_header(oc, opts);
if (ret) {
fprintf(stderr, "Unexpected write_header failure: %s\n",
av_err2str(ret));
goto fail;
}
for (i = 0; i < 15; i++ ) {
ret = prepare_packet(&pkt, pkt_data, i);
if (ret < 0) {
fprintf(stderr, "Failed to prepare test packet: %s\n",
av_err2str(ret));
goto write_trailer_and_fail;
}
ret = av_write_frame(oc, &pkt);
av_packet_unref(&pkt);
if (ret < 0) {
fprintf(stderr, "Unexpected write_frame error: %s\n",
av_err2str(ret));
goto write_trailer_and_fail;
}
}
ret = av_write_frame(oc, NULL);
if (ret < 0) {
fprintf(stderr, "Unexpected write_frame error during flushing: %s\n",
av_err2str(ret));
goto write_trailer_and_fail;
}
ret = av_write_trailer(oc);
if (ret < 0) {
fprintf(stderr, "Unexpected write_trailer error during flushing: %s\n",
av_err2str(ret));
goto fail;
}
return ret;
write_trailer_and_fail:
av_write_trailer(oc);
fail:
return ret;
}
static int fifo_overflow_drop_test(AVFormatContext *oc, AVDictionary **opts,
const FailingMuxerPacketData *data)
{
int ret = 0, i;
int64_t write_pkt_start, write_pkt_end, duration;
AVPacket pkt;
av_init_packet(&pkt);
ret = avformat_write_header(oc, opts);
if (ret) {
fprintf(stderr, "Unexpected write_header failure: %s\n",
av_err2str(ret));
return ret;
}
write_pkt_start = av_gettime_relative();
for (i = 0; i < 6; i++ ) {
ret = prepare_packet(&pkt, data, i);
if (ret < 0) {
fprintf(stderr, "Failed to prepare test packet: %s\n",
av_err2str(ret));
goto fail;
}
ret = av_write_frame(oc, &pkt);
av_packet_unref(&pkt);
if (ret < 0) {
break;
}
}
write_pkt_end = av_gettime_relative();
duration = write_pkt_end - write_pkt_start;
if (duration > (SLEEPTIME_50_MS*6)/2) {
fprintf(stderr, "Writing packets to fifo muxer took too much time while testing"
"buffer overflow with drop_pkts_on_overflow was on.\n");
ret = AVERROR_BUG;
goto fail;
}
if (ret) {
fprintf(stderr, "Unexpected write_packet error: %s\n", av_err2str(ret));
goto fail;
}
ret = av_write_trailer(oc);
if (ret < 0)
fprintf(stderr, "Unexpected write_trailer error: %s\n", av_err2str(ret));
return ret;
fail:
av_write_trailer(oc);
return ret;
}
typedef struct TestCase {
int (*test_func)(AVFormatContext *, AVDictionary **,const FailingMuxerPacketData *pkt_data);
const char *test_name;
const char *options;
uint8_t print_summary_on_deinit;
int write_header_ret;
int write_trailer_ret;
FailingMuxerPacketData pkt_data;
} TestCase;
#define BUFFER_SIZE 64
static int run_test(const TestCase *test)
{
AVDictionary *opts = NULL;
AVFormatContext *oc = NULL;
char buffer[BUFFER_SIZE];
int ret, ret1;
ret = initialize_fifo_tst_muxer_chain(&oc);
if (ret < 0) {
fprintf(stderr, "Muxer initialization failed: %s\n", av_err2str(ret));
goto end;
}
if (test->options) {
ret = av_dict_parse_string(&opts, test->options, "=", ":", 0);
if (ret < 0) {
fprintf(stderr, "Failed to parse options: %s\n", av_err2str(ret));
goto end;
}
}
snprintf(buffer, BUFFER_SIZE,
"print_deinit_summary=%d:write_header_ret=%d:write_trailer_ret=%d",
(int)test->print_summary_on_deinit, test->write_header_ret,
test->write_trailer_ret);
ret = av_dict_set(&opts, "format_opts", buffer, 0);
ret1 = av_dict_set(&opts, "fifo_format", "fifo_test", 0);
if (ret < 0 || ret1 < 0) {
fprintf(stderr, "Failed to set options for test muxer: %s\n",
av_err2str(ret));
goto end;
}
ret = test->test_func(oc, &opts, &test->pkt_data);
end:
printf("%s: %s\n", test->test_name, ret < 0 ? "fail" : "ok");
avformat_free_context(oc);
av_dict_free(&opts);
return ret;
}
const TestCase tests[] = {
{fifo_basic_test, "nonfail test", NULL,1, 0, 0, {0, 0, 0}},
{fifo_basic_test, "recovery test", "attempt_recovery=1:recovery_wait_time=0",
0, 0, 0, {AVERROR(ETIMEDOUT), 3, 0}},
{fifo_basic_test, "overflow without packet dropping","queue_size=3",
1, 0, 0, {0, 0, SLEEPTIME_10_MS}},
{fifo_overflow_drop_test, "overflow with packet dropping", "queue_size=3:drop_pkts_on_overflow=1",
0, 0, 0, {0, 0, SLEEPTIME_50_MS}},
{NULL}
};
int main(int argc, char *argv[])
{
int i, ret, ret_all = 0;
for (i = 0; tests[i].test_func; i++) {
ret = run_test(&tests[i]);
if (!ret_all && ret < 0)
ret_all = ret;
}
return ret;
}
