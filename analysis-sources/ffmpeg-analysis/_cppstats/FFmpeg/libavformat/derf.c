#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"
static int derf_probe(const AVProbeData *p)
{
if (AV_RL32(p->buf) != MKTAG('D','E','R','F'))
return 0;
if (AV_RL32(p->buf+4) != 1 && AV_RL32(p->buf+4) != 2)
return 0;
return AVPROBE_SCORE_MAX / 3 * 2;
}
static int derf_read_header(AVFormatContext *s)
{
unsigned data_size;
AVIOContext *pb = s->pb;
AVCodecParameters *par;
AVStream *st;
avio_skip(pb, 4);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
par = st->codecpar;
par->codec_type = AVMEDIA_TYPE_AUDIO;
par->codec_id = AV_CODEC_ID_DERF_DPCM;
par->format = AV_SAMPLE_FMT_S16;
par->channels = avio_rl32(pb);
if (par->channels != 1 && par->channels != 2)
return AVERROR_INVALIDDATA;
if (par->channels == 1)
par->channel_layout = AV_CH_LAYOUT_MONO;
else if (par->channels == 2)
par->channel_layout = AV_CH_LAYOUT_STEREO;
data_size = avio_rl32(pb);
st->duration = data_size / par->channels;
par->sample_rate = 22050;
par->block_align = 1;
avpriv_set_pts_info(st, 64, 1, par->sample_rate);
return 0;
}
AVInputFormat ff_derf_demuxer = {
.name = "derf",
.long_name = NULL_IF_CONFIG_SMALL("Xilam DERF"),
.read_probe = derf_probe,
.read_header = derf_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
.extensions = "adp",
};
