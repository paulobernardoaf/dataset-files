#include "libavutil/intreadwrite.h"
#include "libavcodec/internal.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"
static int hcom_probe(const AVProbeData *p)
{
if (p->buf_size < 132)
return 0;
if (!memcmp(p->buf+65, "FSSD", 4) &&
!memcmp(p->buf+128, "HCOM", 4))
return AVPROBE_SCORE_MAX;
return 0;
}
static int hcom_read_header(AVFormatContext *s)
{
AVStream *st;
av_unused unsigned data_size, rsrc_size, huffcount;
unsigned compresstype, divisor;
unsigned dict_entries;
int ret;
avio_skip(s->pb, 83);
data_size = avio_rb32(s->pb);
rsrc_size = avio_rb32(s->pb);
avio_skip(s->pb, 128-91+4);
huffcount = avio_rb32(s->pb);
avio_skip(s->pb, 4);
compresstype = avio_rb32(s->pb);
if (compresstype > 1)
return AVERROR_INVALIDDATA;
divisor = avio_rb32(s->pb);
if (divisor == 0 || divisor > 4)
return AVERROR_INVALIDDATA;
dict_entries = avio_rb16(s->pb);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->channels = 1;
st->codecpar->sample_rate = 22050 / divisor;
st->codecpar->codec_id = AV_CODEC_ID_HCOM;
st->codecpar->bits_per_coded_sample = 8;
st->codecpar->block_align = 4;
ret = ff_alloc_extradata(st->codecpar, dict_entries * 4 + 7);
if (ret < 0)
return ret;
AV_WB16(st->codecpar->extradata, dict_entries);
AV_WB32(st->codecpar->extradata + 2, compresstype);
avio_read(s->pb, st->codecpar->extradata + 6, dict_entries * 4);
avio_skip(s->pb, 1);
st->codecpar->extradata[dict_entries * 4 + 6] = avio_r8(s->pb);
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}
AVInputFormat ff_hcom_demuxer = {
.name = "hcom",
.long_name = NULL_IF_CONFIG_SMALL("Macintosh HCOM"),
.read_probe = hcom_probe,
.read_header = hcom_read_header,
.read_packet = ff_pcm_read_packet,
};
