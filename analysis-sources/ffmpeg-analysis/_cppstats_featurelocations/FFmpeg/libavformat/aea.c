





















#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "pcm.h"

#define AT1_SU_SIZE 212

static int aea_read_probe(const AVProbeData *p)
{
if (p->buf_size <= 2048+212)
return 0;


if (AV_RL32(p->buf)==0x800) {
int ch, i;
ch = p->buf[264];

if (ch != 1 && ch != 2)
return 0;





for (i = 2048; i + 211 < p->buf_size; i+= 212) {
int bsm_s, bsm_e, inb_s, inb_e;
bsm_s = p->buf[0];
inb_s = p->buf[1];
inb_e = p->buf[210];
bsm_e = p->buf[211];

if (bsm_s != bsm_e || inb_s != inb_e)
return 0;
}
return AVPROBE_SCORE_MAX / 4 + 1;
}
return 0;
}

static int aea_read_header(AVFormatContext *s)
{
AVStream *st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);


avio_skip(s->pb, 264);
st->codecpar->channels = avio_r8(s->pb);
avio_skip(s->pb, 1783);


st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ATRAC1;
st->codecpar->sample_rate = 44100;
st->codecpar->bit_rate = 292000;

if (st->codecpar->channels != 1 && st->codecpar->channels != 2) {
av_log(s, AV_LOG_ERROR, "Channels %d not supported!\n", st->codecpar->channels);
return AVERROR_INVALIDDATA;
}

st->codecpar->channel_layout = (st->codecpar->channels == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

st->codecpar->block_align = AT1_SU_SIZE * st->codecpar->channels;
return 0;
}

static int aea_read_packet(AVFormatContext *s, AVPacket *pkt)
{
int ret = av_get_packet(s->pb, pkt, s->streams[0]->codecpar->block_align);

pkt->stream_index = 0;
if (ret <= 0)
return AVERROR(EIO);

return ret;
}

AVInputFormat ff_aea_demuxer = {
.name = "aea",
.long_name = NULL_IF_CONFIG_SMALL("MD STUDIO audio"),
.read_probe = aea_read_probe,
.read_header = aea_read_header,
.read_packet = aea_read_packet,
.read_seek = ff_pcm_read_seek,
.flags = AVFMT_GENERIC_INDEX,
.extensions = "aea",
};
