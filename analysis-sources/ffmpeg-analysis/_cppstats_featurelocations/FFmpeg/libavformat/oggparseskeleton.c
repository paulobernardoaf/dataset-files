



















#include "libavcodec/bytestream.h"
#include "avformat.h"
#include "internal.h"
#include "oggdec.h"

static int skeleton_header(AVFormatContext *s, int idx)
{
struct ogg *ogg = s->priv_data;
struct ogg_stream *os = ogg->streams + idx;
AVStream *st = s->streams[idx];
uint8_t *buf = os->buf + os->pstart;
int version_major, version_minor;
int64_t start_num, start_den;
uint64_t start_granule;
int target_idx, start_time;

st->codecpar->codec_type = AVMEDIA_TYPE_DATA;

if ((os->flags & OGG_FLAG_EOS) && os->psize == 0)
return 1;

if (os->psize < 8)
return -1;

if (!strncmp(buf, "fishead", 8)) {
if (os->psize < 64)
return -1;

version_major = AV_RL16(buf+8);
version_minor = AV_RL16(buf+10);

if (version_major != 3 && version_major != 4) {
av_log(s, AV_LOG_WARNING, "Unknown skeleton version %d.%d\n",
version_major, version_minor);
return -1;
}






start_num = AV_RL64(buf+12);
start_den = AV_RL64(buf+20);

if (start_den > 0 && start_num > 0) {
int base_den;
av_reduce(&start_time, &base_den, start_num, start_den, INT_MAX);
avpriv_set_pts_info(st, 64, 1, base_den);
os->lastpts =
st->start_time = start_time;
}
} else if (!strncmp(buf, "fisbone", 8)) {
if (os->psize < 52)
return -1;

target_idx = ogg_find_stream(ogg, AV_RL32(buf+12));
start_granule = AV_RL64(buf+36);
if (target_idx < 0) {
av_log(s, AV_LOG_WARNING, "Serial number in fisbone doesn't match any stream\n");
return 1;
}
os = ogg->streams + target_idx;
if (os->start_granule != OGG_NOGRANULE_VALUE) {
av_log(s, AV_LOG_WARNING, "Multiple fisbone for the same stream\n");
return 1;
}
if (start_granule != OGG_NOGRANULE_VALUE) {
os->start_granule = start_granule;
}
}

return 1;
}

const struct ogg_codec ff_skeleton_codec = {
.magic = "fishead",
.magicsize = 8,
.header = skeleton_header,
.nb_header = 0,
};
