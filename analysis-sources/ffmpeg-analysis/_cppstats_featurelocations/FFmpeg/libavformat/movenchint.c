




















#include "movenc.h"
#include "libavutil/intreadwrite.h"
#include "internal.h"
#include "rtpenc_chain.h"
#include "avio_internal.h"
#include "rtp.h"

int ff_mov_init_hinting(AVFormatContext *s, int index, int src_index)
{
MOVMuxContext *mov = s->priv_data;
MOVTrack *track = &mov->tracks[index];
MOVTrack *src_track = &mov->tracks[src_index];
AVStream *src_st = s->streams[src_index];
int ret = AVERROR(ENOMEM);

track->tag = MKTAG('r','t','p',' ');
track->src_track = src_index;

track->par = avcodec_parameters_alloc();
if (!track->par)
goto fail;
track->par->codec_type = AVMEDIA_TYPE_DATA;
track->par->codec_tag = track->tag;

ret = ff_rtp_chain_mux_open(&track->rtp_ctx, s, src_st, NULL,
RTP_MAX_PACKET_SIZE, src_index);
if (ret < 0)
goto fail;


track->timescale = track->rtp_ctx->streams[0]->time_base.den;



src_track->hint_track = index;
return 0;
fail:
av_log(s, AV_LOG_WARNING,
"Unable to initialize hinting of stream %d\n", src_index);
avcodec_parameters_free(&track->par);

track->timescale = 90000;
return ret;
}




static void sample_queue_pop(HintSampleQueue *queue)
{
if (queue->len <= 0)
return;
if (queue->samples[0].own_data)
av_freep(&queue->samples[0].data);
queue->len--;
memmove(queue->samples, queue->samples + 1, sizeof(HintSample)*queue->len);
}




static void sample_queue_free(HintSampleQueue *queue)
{
int i;
for (i = 0; i < queue->len; i++)
if (queue->samples[i].own_data)
av_freep(&queue->samples[i].data);
av_freep(&queue->samples);
queue->len = 0;
queue->size = 0;
}






static void sample_queue_push(HintSampleQueue *queue, uint8_t *data, int size,
int sample)
{


if (size <= 14)
return;
if (!queue->samples || queue->len >= queue->size) {
HintSample *samples;
samples = av_realloc_array(queue->samples, queue->size + 10, sizeof(HintSample));
if (!samples)
return;
queue->size += 10;
queue->samples = samples;
}
queue->samples[queue->len].data = data;
queue->samples[queue->len].size = size;
queue->samples[queue->len].sample_number = sample;
queue->samples[queue->len].offset = 0;
queue->samples[queue->len].own_data = 0;
queue->len++;
}




static void sample_queue_retain(HintSampleQueue *queue)
{
int i;
for (i = 0; i < queue->len; ) {
HintSample *sample = &queue->samples[i];
if (!sample->own_data) {
uint8_t *ptr = av_malloc(sample->size);
if (!ptr) {

memmove(queue->samples + i, queue->samples + i + 1,
sizeof(HintSample)*(queue->len - i - 1));
queue->len--;
continue;
}
memcpy(ptr, sample->data, sample->size);
sample->data = ptr;
sample->own_data = 1;
}
i++;
}
}

















static int match_segments(const uint8_t *haystack, int h_len,
const uint8_t *needle, int n_pos, int n_len,
int *match_h_offset_ptr, int *match_n_offset_ptr,
int *match_len_ptr)
{
int h_pos;
for (h_pos = 0; h_pos < h_len; h_pos++) {
int match_len = 0;
int match_h_pos, match_n_pos;


while (h_pos + match_len < h_len && n_pos + match_len < n_len &&
needle[n_pos + match_len] == haystack[h_pos + match_len])
match_len++;
if (match_len <= 8)
continue;



match_h_pos = h_pos;
match_n_pos = n_pos;
while (match_n_pos > 0 && match_h_pos > 0 &&
needle[match_n_pos - 1] == haystack[match_h_pos - 1]) {
match_n_pos--;
match_h_pos--;
match_len++;
}
if (match_len <= 14)
continue;
*match_h_offset_ptr = match_h_pos;
*match_n_offset_ptr = match_n_pos;
*match_len_ptr = match_len;
return 0;
}
return -1;
}
















static int find_sample_match(const uint8_t *data, int len,
HintSampleQueue *queue, int *pos,
int *match_sample, int *match_offset,
int *match_len)
{
while (queue->len > 0) {
HintSample *sample = &queue->samples[0];


if (sample->offset == 0 && sample->size > 5)
sample->offset = 5;

if (match_segments(data, len, sample->data, sample->offset,
sample->size, pos, match_offset, match_len) == 0) {
*match_sample = sample->sample_number;


sample->offset = *match_offset + *match_len + 5;
if (sample->offset + 10 >= sample->size)
sample_queue_pop(queue); 
return 0;
}

if (sample->offset < 10 && sample->size > 20) {


sample->offset = sample->size/2;
} else {

sample_queue_pop(queue);
}
}
return -1;
}

static void output_immediate(const uint8_t *data, int size,
AVIOContext *out, int *entries)
{
while (size > 0) {
int len = size;
if (len > 14)
len = 14;
avio_w8(out, 1); 
avio_w8(out, len); 
avio_write(out, data, len);
data += len;
size -= len;

for (; len < 14; len++)
avio_w8(out, 0);

(*entries)++;
}
}

static void output_match(AVIOContext *out, int match_sample,
int match_offset, int match_len, int *entries)
{
avio_w8(out, 2); 
avio_w8(out, 0); 
avio_wb16(out, match_len);
avio_wb32(out, match_sample);
avio_wb32(out, match_offset);
avio_wb16(out, 1); 
avio_wb16(out, 1); 
(*entries)++;
}

static void describe_payload(const uint8_t *data, int size,
AVIOContext *out, int *entries,
HintSampleQueue *queue)
{

while (size > 0) {
int match_sample, match_offset, match_len, pos;
if (find_sample_match(data, size, queue, &pos, &match_sample,
&match_offset, &match_len) < 0)
break;
output_immediate(data, pos, out, entries);
data += pos;
size -= pos;
output_match(out, match_sample, match_offset, match_len, entries);
data += match_len;
size -= match_len;
}
output_immediate(data, size, out, entries);
}













static int write_hint_packets(AVIOContext *out, const uint8_t *data,
int size, MOVTrack *trk, int64_t *dts)
{
int64_t curpos;
int64_t count_pos, entries_pos;
int count = 0, entries;

count_pos = avio_tell(out);

avio_wb16(out, 0); 
avio_wb16(out, 0); 

while (size > 4) {
uint32_t packet_len = AV_RB32(data);
uint16_t seq;
uint32_t ts;
int32_t ts_diff;

data += 4;
size -= 4;
if (packet_len > size || packet_len <= 12)
break;
if (RTP_PT_IS_RTCP(data[1])) {

data += packet_len;
size -= packet_len;
continue;
}

if (packet_len > trk->max_packet_size)
trk->max_packet_size = packet_len;

seq = AV_RB16(&data[2]);
ts = AV_RB32(&data[4]);

if (trk->prev_rtp_ts == 0)
trk->prev_rtp_ts = ts;


ts_diff = ts - trk->prev_rtp_ts;
if (ts_diff > 0) {
trk->cur_rtp_ts_unwrapped += ts_diff;
trk->prev_rtp_ts = ts;
ts_diff = 0;
}
if (*dts == AV_NOPTS_VALUE)
*dts = trk->cur_rtp_ts_unwrapped;

count++;

avio_wb32(out, 0); 
avio_write(out, data, 2); 
avio_wb16(out, seq); 
avio_wb16(out, ts_diff ? 4 : 0); 
entries_pos = avio_tell(out);
avio_wb16(out, 0); 
if (ts_diff) { 
avio_wb32(out, 16); 
avio_wb32(out, 12); 
avio_write(out, "rtpo", 4);
avio_wb32(out, ts_diff);
}

data += 12;
size -= 12;
packet_len -= 12;

entries = 0;

describe_payload(data, packet_len, out, &entries, &trk->sample_queue);
data += packet_len;
size -= packet_len;

curpos = avio_tell(out);
avio_seek(out, entries_pos, SEEK_SET);
avio_wb16(out, entries);
avio_seek(out, curpos, SEEK_SET);
}

curpos = avio_tell(out);
avio_seek(out, count_pos, SEEK_SET);
avio_wb16(out, count);
avio_seek(out, curpos, SEEK_SET);
return count;
}

int ff_mov_add_hinted_packet(AVFormatContext *s, AVPacket *pkt,
int track_index, int sample,
uint8_t *sample_data, int sample_size)
{
MOVMuxContext *mov = s->priv_data;
MOVTrack *trk = &mov->tracks[track_index];
AVFormatContext *rtp_ctx = trk->rtp_ctx;
uint8_t *buf = NULL;
int size;
AVIOContext *hintbuf = NULL;
AVPacket hint_pkt;
int ret = 0, count;

if (!rtp_ctx)
return AVERROR(ENOENT);
if (!rtp_ctx->pb)
return AVERROR(ENOMEM);

if (sample_data)
sample_queue_push(&trk->sample_queue, sample_data, sample_size, sample);
else
sample_queue_push(&trk->sample_queue, pkt->data, pkt->size, sample);


ff_write_chained(rtp_ctx, 0, pkt, s, 0);



size = avio_close_dyn_buf(rtp_ctx->pb, &buf);
if ((ret = ffio_open_dyn_packet_buf(&rtp_ctx->pb,
RTP_MAX_PACKET_SIZE)) < 0)
goto done;

if (size <= 0)
goto done;


if ((ret = avio_open_dyn_buf(&hintbuf)) < 0)
goto done;
av_init_packet(&hint_pkt);
count = write_hint_packets(hintbuf, buf, size, trk, &hint_pkt.dts);
av_freep(&buf);


hint_pkt.size = size = avio_close_dyn_buf(hintbuf, &buf);
hint_pkt.data = buf;
hint_pkt.pts = hint_pkt.dts;
hint_pkt.stream_index = track_index;
if (pkt->flags & AV_PKT_FLAG_KEY)
hint_pkt.flags |= AV_PKT_FLAG_KEY;
if (count > 0)
ff_mov_write_packet(s, &hint_pkt);
done:
av_free(buf);
sample_queue_retain(&trk->sample_queue);
return ret;
}

void ff_mov_close_hinting(MOVTrack *track)
{
AVFormatContext *rtp_ctx = track->rtp_ctx;

avcodec_parameters_free(&track->par);
sample_queue_free(&track->sample_queue);
if (!rtp_ctx)
return;
if (rtp_ctx->pb) {
av_write_trailer(rtp_ctx);
ffio_free_dyn_buf(&rtp_ctx->pb);
}
avformat_free_context(rtp_ctx);
}
