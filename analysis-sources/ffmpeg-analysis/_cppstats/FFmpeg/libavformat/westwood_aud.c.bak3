


































#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

#define AUD_HEADER_SIZE 12
#define AUD_CHUNK_PREAMBLE_SIZE 8
#define AUD_CHUNK_SIGNATURE 0x0000DEAF

static int wsaud_probe(const AVProbeData *p)
{
int field;











if (p->buf_size < AUD_HEADER_SIZE + AUD_CHUNK_PREAMBLE_SIZE)
return 0;


field = AV_RL16(&p->buf[0]);
if ((field < 8000) || (field > 48000))
return 0;



if (p->buf[10] & 0xFC)
return 0;

if (p->buf[11] != 99 && p->buf[11] != 1)
return 0;


if (AV_RL32(&p->buf[16]) != AUD_CHUNK_SIGNATURE)
return 0;


return AVPROBE_SCORE_EXTENSION;
}

static int wsaud_read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
AVStream *st;
unsigned char header[AUD_HEADER_SIZE];
int sample_rate, channels, codec;

if (avio_read(pb, header, AUD_HEADER_SIZE) != AUD_HEADER_SIZE)
return AVERROR(EIO);

sample_rate = AV_RL16(&header[0]);
channels = (header[10] & 0x1) + 1;
codec = header[11];


st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

switch (codec) {
case 1:
if (channels != 1) {
avpriv_request_sample(s, "Stereo WS-SND1");
return AVERROR_PATCHWELCOME;
}
st->codecpar->codec_id = AV_CODEC_ID_WESTWOOD_SND1;
break;
case 99:
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_IMA_WS;
st->codecpar->bits_per_coded_sample = 4;
st->codecpar->bit_rate = channels * sample_rate * 4;
break;
default:
avpriv_request_sample(s, "Unknown codec: %d", codec);
return AVERROR_PATCHWELCOME;
}
avpriv_set_pts_info(st, 64, 1, sample_rate);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->channels = channels;
st->codecpar->channel_layout = channels == 1 ? AV_CH_LAYOUT_MONO :
AV_CH_LAYOUT_STEREO;
st->codecpar->sample_rate = sample_rate;

return 0;
}

static int wsaud_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
AVIOContext *pb = s->pb;
unsigned char preamble[AUD_CHUNK_PREAMBLE_SIZE];
unsigned int chunk_size;
int ret = 0;
AVStream *st = s->streams[0];

if (avio_read(pb, preamble, AUD_CHUNK_PREAMBLE_SIZE) !=
AUD_CHUNK_PREAMBLE_SIZE)
return AVERROR(EIO);


if (AV_RL32(&preamble[4]) != AUD_CHUNK_SIGNATURE)
return AVERROR_INVALIDDATA;

chunk_size = AV_RL16(&preamble[0]);

if (st->codecpar->codec_id == AV_CODEC_ID_WESTWOOD_SND1) {




int out_size = AV_RL16(&preamble[2]);
if ((ret = av_new_packet(pkt, chunk_size + 4)) < 0)
return ret;
if ((ret = avio_read(pb, &pkt->data[4], chunk_size)) != chunk_size)
return ret < 0 ? ret : AVERROR(EIO);
AV_WL16(&pkt->data[0], out_size);
AV_WL16(&pkt->data[2], chunk_size);

pkt->duration = out_size;
} else {
ret = av_get_packet(pb, pkt, chunk_size);
if (ret != chunk_size)
return AVERROR(EIO);

if (st->codecpar->channels <= 0) {
av_log(s, AV_LOG_ERROR, "invalid number of channels %d\n",
st->codecpar->channels);
return AVERROR_INVALIDDATA;
}


pkt->duration = (chunk_size * 2) / st->codecpar->channels;
}
pkt->stream_index = st->index;

return ret;
}

AVInputFormat ff_wsaud_demuxer = {
.name = "wsaud",
.long_name = NULL_IF_CONFIG_SMALL("Westwood Studios audio"),
.read_probe = wsaud_probe,
.read_header = wsaud_read_header,
.read_packet = wsaud_read_packet,
};
