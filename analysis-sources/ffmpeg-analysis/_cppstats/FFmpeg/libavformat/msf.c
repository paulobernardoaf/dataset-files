#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
static int msf_probe(const AVProbeData *p)
{
if (memcmp(p->buf, "MSF", 3))
return 0;
if (AV_RB32(p->buf+8) <= 0)
return 0;
if (AV_RB32(p->buf+16) <= 0)
return 0;
if (AV_RB32(p->buf+4) > 16)
return AVPROBE_SCORE_MAX / 5; 
return AVPROBE_SCORE_MAX / 3 * 2;
}
static int msf_read_header(AVFormatContext *s)
{
unsigned codec, size;
AVStream *st;
int ret;
avio_skip(s->pb, 4);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
codec = avio_rb32(s->pb);
st->codecpar->channels = avio_rb32(s->pb);
if (st->codecpar->channels <= 0 || st->codecpar->channels >= INT_MAX / 1024)
return AVERROR_INVALIDDATA;
size = avio_rb32(s->pb);
st->codecpar->sample_rate = avio_rb32(s->pb);
if (st->codecpar->sample_rate <= 0)
return AVERROR_INVALIDDATA;
switch (codec) {
case 0: st->codecpar->codec_id = AV_CODEC_ID_PCM_S16BE; break;
case 1: st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE; break;
case 3: st->codecpar->block_align = 16 * st->codecpar->channels;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_PSX; break;
case 4:
case 5:
case 6: st->codecpar->block_align = (codec == 4 ? 96 : codec == 5 ? 152 : 192) * st->codecpar->channels;
ret = ff_alloc_extradata(st->codecpar, 14);
if (ret < 0)
return ret;
memset(st->codecpar->extradata, 0, st->codecpar->extradata_size);
AV_WL16(st->codecpar->extradata, 1); 
AV_WL16(st->codecpar->extradata+2, 2048 * st->codecpar->channels); 
AV_WL16(st->codecpar->extradata+6, codec == 4 ? 1 : 0); 
AV_WL16(st->codecpar->extradata+8, codec == 4 ? 1 : 0); 
AV_WL16(st->codecpar->extradata+10, 1);
st->codecpar->codec_id = AV_CODEC_ID_ATRAC3; break;
case 7: st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
st->codecpar->codec_id = AV_CODEC_ID_MP3; break;
default:
avpriv_request_sample(s, "Codec %d", codec);
return AVERROR_PATCHWELCOME;
}
st->duration = av_get_audio_frame_duration2(st->codecpar, size);
avio_skip(s->pb, 0x40 - avio_tell(s->pb));
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}
static int msf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVCodecParameters *par = s->streams[0]->codecpar;
return av_get_packet(s->pb, pkt, par->block_align ? par->block_align : 1024 * par->channels);
}
AVInputFormat ff_msf_demuxer = {
.name = "msf",
.long_name = NULL_IF_CONFIG_SMALL("Sony PS3 MSF"),
.read_probe = msf_probe,
.read_header = msf_read_header,
.read_packet = msf_read_packet,
.extensions = "msf",
};
