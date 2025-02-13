




















#include <inttypes.h>
#include <stdint.h>

#include "avformat.h"
#include "internal.h"
#include "riff.h"





typedef struct XWMAContext {
int64_t data_end;
} XWMAContext;

static int xwma_probe(const AVProbeData *p)
{
if (!memcmp(p->buf, "RIFF", 4) && !memcmp(p->buf + 8, "XWMA", 4))
return AVPROBE_SCORE_MAX;
return 0;
}

static int xwma_read_header(AVFormatContext *s)
{
int64_t size;
int ret = 0;
uint32_t dpds_table_size = 0;
uint32_t *dpds_table = NULL;
unsigned int tag;
AVIOContext *pb = s->pb;
AVStream *st;
XWMAContext *xwma = s->priv_data;
int i;






tag = avio_rl32(pb);
if (tag != MKTAG('R', 'I', 'F', 'F'))
return AVERROR_INVALIDDATA;
avio_rl32(pb); 
tag = avio_rl32(pb);
if (tag != MKTAG('X', 'W', 'M', 'A'))
return AVERROR_INVALIDDATA;


tag = avio_rl32(pb);
if (tag != MKTAG('f', 'm', 't', ' '))
return AVERROR_INVALIDDATA;
size = avio_rl32(pb);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

ret = ff_get_wav_header(s, pb, st->codecpar, size, 0);
if (ret < 0)
return ret;
st->need_parsing = AVSTREAM_PARSE_NONE;





if (st->codecpar->codec_id == AV_CODEC_ID_WMAV2) {
int ch = st->codecpar->channels;
int sr = st->codecpar->sample_rate;
int br = st->codecpar->bit_rate;

if (ch == 1) {
if (sr == 22050 && (br==48000 || br==192000))
br = 20000;
else if (sr == 32000 && (br==48000 || br==192000))
br = 20000;
else if (sr == 44100 && (br==96000 || br==192000))
br = 48000;
}
else if (ch == 2) {
if (sr == 22050 && (br==48000 || br==192000))
br = 32000;
else if (sr == 32000 && (br==192000))
br = 48000;
}

st->codecpar->bit_rate = br;
}



if (st->codecpar->codec_id != AV_CODEC_ID_WMAV2 &&
st->codecpar->codec_id != AV_CODEC_ID_WMAPRO) {
avpriv_request_sample(s, "Unexpected codec (tag %s; id %d)",
av_fourcc2str(st->codecpar->codec_tag),
st->codecpar->codec_id);
} else {







if (st->codecpar->extradata_size != 0) {




avpriv_request_sample(s, "Unexpected extradata (%d bytes)",
st->codecpar->extradata_size);
} else if (st->codecpar->codec_id == AV_CODEC_ID_WMAPRO) {
if ((ret = ff_alloc_extradata(st->codecpar, 18)) < 0)
return ret;

memset(st->codecpar->extradata, 0, st->codecpar->extradata_size);
st->codecpar->extradata[ 0] = st->codecpar->bits_per_coded_sample;
st->codecpar->extradata[14] = 224;
} else {
if ((ret = ff_alloc_extradata(st->codecpar, 6)) < 0)
return ret;

memset(st->codecpar->extradata, 0, st->codecpar->extradata_size);

st->codecpar->extradata[4] = 31;
}
}

if (!st->codecpar->channels) {
av_log(s, AV_LOG_WARNING, "Invalid channel count: %d\n",
st->codecpar->channels);
return AVERROR_INVALIDDATA;
}
if (!st->codecpar->bits_per_coded_sample) {
av_log(s, AV_LOG_WARNING, "Invalid bits_per_coded_sample: %d\n",
st->codecpar->bits_per_coded_sample);
return AVERROR_INVALIDDATA;
}


avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);


for (;;) {
if (pb->eof_reached) {
ret = AVERROR_EOF;
goto fail;
}

tag = avio_rl32(pb);
size = avio_rl32(pb);
if (tag == MKTAG('d', 'a', 't', 'a')) {

break;
} else if (tag == MKTAG('d','p','d','s')) {











if (dpds_table) {
av_log(s, AV_LOG_ERROR, "two dpds chunks present\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}


if (size & 3) { 
av_log(s, AV_LOG_WARNING,
"dpds chunk size %"PRId64" not divisible by 4\n", size);
}
dpds_table_size = size / 4;
if (dpds_table_size == 0 || dpds_table_size >= INT_MAX / 4) {
av_log(s, AV_LOG_ERROR,
"dpds chunk size %"PRId64" invalid\n", size);
return AVERROR_INVALIDDATA;
}




dpds_table = av_malloc_array(dpds_table_size, sizeof(uint32_t));
if (!dpds_table) {
return AVERROR(ENOMEM);
}

for (i = 0; i < dpds_table_size; ++i) {
dpds_table[i] = avio_rl32(pb);
size -= 4;
}
}
avio_skip(pb, size);
}


if (size < 0) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
if (!size) {
xwma->data_end = INT64_MAX;
} else
xwma->data_end = avio_tell(pb) + size;


if (dpds_table && dpds_table_size) {
int64_t cur_pos;
const uint32_t bytes_per_sample
= (st->codecpar->channels * st->codecpar->bits_per_coded_sample) >> 3;


const uint64_t total_decoded_bytes = dpds_table[dpds_table_size - 1];

if (!bytes_per_sample) {
av_log(s, AV_LOG_ERROR,
"Invalid bits_per_coded_sample %d for %d channels\n",
st->codecpar->bits_per_coded_sample, st->codecpar->channels);
ret = AVERROR_INVALIDDATA;
goto fail;
}

st->duration = total_decoded_bytes / bytes_per_sample;










cur_pos = avio_tell(pb);
for (i = 0; i < dpds_table_size; ++i) {




av_add_index_entry(st,
cur_pos + (i+1) * st->codecpar->block_align, 
dpds_table[i] / bytes_per_sample, 
st->codecpar->block_align, 
0, 
AVINDEX_KEYFRAME);
}
} else if (st->codecpar->bit_rate) {




st->duration = (size<<3) * st->codecpar->sample_rate / st->codecpar->bit_rate;
}

fail:
av_free(dpds_table);

return ret;
}

static int xwma_read_packet(AVFormatContext *s, AVPacket *pkt)
{
int ret, size;
int64_t left;
AVStream *st;
XWMAContext *xwma = s->priv_data;

st = s->streams[0];

left = xwma->data_end - avio_tell(s->pb);
if (left <= 0) {
return AVERROR_EOF;
}


size = (st->codecpar->block_align > 1) ? st->codecpar->block_align : 2230;
size = FFMIN(size, left);

ret = av_get_packet(s->pb, pkt, size);
if (ret < 0)
return ret;

pkt->stream_index = 0;
return ret;
}

AVInputFormat ff_xwma_demuxer = {
.name = "xwma",
.long_name = NULL_IF_CONFIG_SMALL("Microsoft xWMA"),
.priv_data_size = sizeof(XWMAContext),
.read_probe = xwma_probe,
.read_header = xwma_read_header,
.read_packet = xwma_read_packet,
};
