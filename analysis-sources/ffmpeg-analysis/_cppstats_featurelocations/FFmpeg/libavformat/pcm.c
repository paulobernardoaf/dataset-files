




















#include "libavutil/mathematics.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"

#define RAW_SAMPLES 1024

int ff_pcm_read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVCodecParameters *par = s->streams[0]->codecpar;
int ret, size;

if (par->block_align <= 0)
return AVERROR(EINVAL);





size = FFMAX(par->sample_rate/25, 1);
size = FFMIN(size, RAW_SAMPLES) * par->block_align;

ret = av_get_packet(s->pb, pkt, size);

pkt->flags &= ~AV_PKT_FLAG_CORRUPT;
pkt->stream_index = 0;

return ret;
}

int ff_pcm_read_seek(AVFormatContext *s,
int stream_index, int64_t timestamp, int flags)
{
AVStream *st;
int block_align, byte_rate;
int64_t pos, ret;

st = s->streams[0];

block_align = st->codecpar->block_align ? st->codecpar->block_align :
(av_get_bits_per_sample(st->codecpar->codec_id) * st->codecpar->channels) >> 3;
byte_rate = st->codecpar->bit_rate ? st->codecpar->bit_rate >> 3 :
block_align * st->codecpar->sample_rate;

if (block_align <= 0 || byte_rate <= 0)
return -1;
if (timestamp < 0) timestamp = 0;


pos = av_rescale_rnd(timestamp * byte_rate,
st->time_base.num,
st->time_base.den * (int64_t)block_align,
(flags & AVSEEK_FLAG_BACKWARD) ? AV_ROUND_DOWN : AV_ROUND_UP);
pos *= block_align;


st->cur_dts = av_rescale(pos, st->time_base.den, byte_rate * (int64_t)st->time_base.num);
if ((ret = avio_seek(s->pb, pos + s->internal->data_offset, SEEK_SET)) < 0)
return ret;
return 0;
}
