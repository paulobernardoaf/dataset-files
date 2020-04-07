




















#include <string.h>

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "oggdec.h"

struct oggcelt_private {
int extra_headers_left;
};

static int celt_header(AVFormatContext *s, int idx)
{
struct ogg *ogg = s->priv_data;
struct ogg_stream *os = ogg->streams + idx;
AVStream *st = s->streams[idx];
struct oggcelt_private *priv = os->private;
uint8_t *p = os->buf + os->pstart;
int ret;

if (os->psize == 60 &&
!memcmp(p, ff_celt_codec.magic, ff_celt_codec.magicsize)) {


uint32_t version, sample_rate, nb_channels;
uint32_t overlap, extra_headers;

priv = av_malloc(sizeof(struct oggcelt_private));
if (!priv)
return AVERROR(ENOMEM);
ret = ff_alloc_extradata(st->codecpar, 2 * sizeof(uint32_t));
if (ret < 0) {
av_free(priv);
return ret;
}
version = AV_RL32(p + 28);

sample_rate = AV_RL32(p + 36);
nb_channels = AV_RL32(p + 40);
overlap = AV_RL32(p + 48);

extra_headers = AV_RL32(p + 56);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_CELT;
st->codecpar->sample_rate = sample_rate;
st->codecpar->channels = nb_channels;
if (sample_rate)
avpriv_set_pts_info(st, 64, 1, sample_rate);

if (os->private) {
av_free(priv);
priv = os->private;
}
os->private = priv;
priv->extra_headers_left = 1 + extra_headers;

AV_WL32(st->codecpar->extradata + 0, overlap);
AV_WL32(st->codecpar->extradata + 4, version);
return 1;
} else if (priv && priv->extra_headers_left) {


ff_vorbis_stream_comment(s, st, p, os->psize);
priv->extra_headers_left--;
return 1;
} else {
return 0;
}
}

const struct ogg_codec ff_celt_codec = {
.magic = "CELT ",
.magicsize = 8,
.header = celt_header,
.nb_header = 2,
};
