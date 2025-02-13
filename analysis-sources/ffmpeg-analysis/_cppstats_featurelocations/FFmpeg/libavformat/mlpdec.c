






















#include "avformat.h"
#include "rawdec.h"
#include "libavutil/intreadwrite.h"

static int av_always_inline mlp_thd_probe(const AVProbeData *p, uint32_t sync)
{
const uint8_t *buf, *last_buf = p->buf, *end = p->buf + p->buf_size;
int frames = 0, valid = 0, size = 0;
int nsubframes = 0;

for (buf = p->buf; buf + 8 <= end; buf++) {
if (AV_RB32(buf + 4) == sync) {
frames++;
if (last_buf + size == buf) {
valid += 1 + nsubframes / 8;
}
nsubframes = 0;
last_buf = buf;
size = (AV_RB16(buf) & 0xfff) * 2;
} else if (buf - last_buf == size) {
nsubframes++;
size += (AV_RB16(buf) & 0xfff) * 2;
}
}
if (valid >= 100)
return AVPROBE_SCORE_MAX;
return 0;
}

#if CONFIG_MLP_DEMUXER
static int mlp_probe(const AVProbeData *p)
{
return mlp_thd_probe(p, 0xf8726fbb);
}

FF_RAW_DEMUXER_CLASS(mlp)
AVInputFormat ff_mlp_demuxer = {
.name = "mlp",
.long_name = NULL_IF_CONFIG_SMALL("raw MLP"),
.read_probe = mlp_probe,
.read_header = ff_raw_audio_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags = AVFMT_GENERIC_INDEX | AVFMT_NOTIMESTAMPS,
.extensions = "mlp",
.raw_codec_id = AV_CODEC_ID_MLP,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &mlp_demuxer_class,
};
#endif

#if CONFIG_TRUEHD_DEMUXER
static int thd_probe(const AVProbeData *p)
{
return mlp_thd_probe(p, 0xf8726fba);
}

FF_RAW_DEMUXER_CLASS(truehd)
AVInputFormat ff_truehd_demuxer = {
.name = "truehd",
.long_name = NULL_IF_CONFIG_SMALL("raw TrueHD"),
.read_probe = thd_probe,
.read_header = ff_raw_audio_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags = AVFMT_GENERIC_INDEX | AVFMT_NOTIMESTAMPS,
.extensions = "thd",
.raw_codec_id = AV_CODEC_ID_TRUEHD,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &truehd_demuxer_class,
};
#endif

