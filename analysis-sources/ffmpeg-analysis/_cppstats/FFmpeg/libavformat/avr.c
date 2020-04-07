#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"
static int avr_probe(const AVProbeData *p)
{
if (AV_RL32(p->buf) != MKTAG('2', 'B', 'I', 'T'))
return 0;
if (!AV_RB16(p->buf+12) || AV_RB16(p->buf+12) > 256) 
return AVPROBE_SCORE_EXTENSION/2;
if (AV_RB16(p->buf+14) > 256) 
return AVPROBE_SCORE_EXTENSION/2;
return AVPROBE_SCORE_EXTENSION;
}
static int avr_read_header(AVFormatContext *s)
{
uint16_t chan, sign, bps;
AVStream *st;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
avio_skip(s->pb, 4); 
avio_skip(s->pb, 8); 
chan = avio_rb16(s->pb);
if (!chan) {
st->codecpar->channels = 1;
} else if (chan == 0xFFFFu) {
st->codecpar->channels = 2;
} else {
avpriv_request_sample(s, "chan %d", chan);
return AVERROR_PATCHWELCOME;
}
st->codecpar->bits_per_coded_sample = bps = avio_rb16(s->pb);
sign = avio_rb16(s->pb);
avio_skip(s->pb, 2); 
avio_skip(s->pb, 2); 
avio_skip(s->pb, 1); 
st->codecpar->sample_rate = avio_rb24(s->pb);
avio_skip(s->pb, 4 * 3);
avio_skip(s->pb, 2 * 3);
avio_skip(s->pb, 20);
avio_skip(s->pb, 64);
st->codecpar->codec_id = ff_get_pcm_codec_id(bps, 0, 1, sign);
if (st->codecpar->codec_id == AV_CODEC_ID_NONE) {
avpriv_request_sample(s, "Bps %d and sign %d", bps, sign);
return AVERROR_PATCHWELCOME;
}
st->codecpar->block_align = bps * st->codecpar->channels / 8;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}
AVInputFormat ff_avr_demuxer = {
.name = "avr",
.long_name = NULL_IF_CONFIG_SMALL("AVR (Audio Visual Research)"),
.read_probe = avr_probe,
.read_header = avr_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
.extensions = "avr",
.flags = AVFMT_GENERIC_INDEX,
};
