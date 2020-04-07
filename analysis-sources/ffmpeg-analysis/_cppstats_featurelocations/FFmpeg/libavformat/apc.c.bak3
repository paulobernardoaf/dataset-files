




















#include <string.h>

#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "internal.h"

static int apc_probe(const AVProbeData *p)
{
if (!strncmp(p->buf, "CRYO_APC", 8))
return AVPROBE_SCORE_MAX;

return 0;
}

static int apc_read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
AVStream *st;
int ret;

avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_IMA_APC;

avio_rl32(pb); 
st->codecpar->sample_rate = avio_rl32(pb);


if ((ret = ff_get_extradata(s, st->codecpar, pb, 2 * 4)) < 0)
return ret;

if (avio_rl32(pb)) {
st->codecpar->channels = 2;
st->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
} else {
st->codecpar->channels = 1;
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
}

st->codecpar->bits_per_coded_sample = 4;
st->codecpar->bit_rate = (int64_t)st->codecpar->bits_per_coded_sample * st->codecpar->channels
* st->codecpar->sample_rate;
st->codecpar->block_align = 1;

return 0;
}

#define MAX_READ_SIZE 4096

static int apc_read_packet(AVFormatContext *s, AVPacket *pkt)
{
if (av_get_packet(s->pb, pkt, MAX_READ_SIZE) <= 0)
return AVERROR(EIO);
pkt->stream_index = 0;
return 0;
}

AVInputFormat ff_apc_demuxer = {
.name = "apc",
.long_name = NULL_IF_CONFIG_SMALL("CRYO APC"),
.read_probe = apc_probe,
.read_header = apc_read_header,
.read_packet = apc_read_packet,
};
