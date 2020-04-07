#include "libavutil/intreadwrite.h"
#include "libavutil/internal.h"
#include "avformat.h"
#include "internal.h"
#include "rawdec.h"
#define LOAS_SYNC_WORD 0x2b7
static int loas_probe(const AVProbeData *p)
{
int max_frames = 0, first_frames = 0;
int fsize, frames;
const uint8_t *buf0 = p->buf;
const uint8_t *buf2;
const uint8_t *buf;
const uint8_t *end = buf0 + p->buf_size - 3;
buf = buf0;
for (; buf < end; buf = buf2 + 1) {
buf2 = buf;
for (frames = 0; buf2 < end; frames++) {
uint32_t header = AV_RB24(buf2);
if ((header >> 13) != LOAS_SYNC_WORD)
break;
fsize = (header & 0x1FFF) + 3;
if (fsize < 7)
break;
fsize = FFMIN(fsize, end - buf2);
buf2 += fsize;
}
max_frames = FFMAX(max_frames, frames);
if (buf == buf0)
first_frames = frames;
}
if (first_frames >= 3)
return AVPROBE_SCORE_EXTENSION + 1;
else if (max_frames > 100)
return AVPROBE_SCORE_EXTENSION;
else if (max_frames >= 3)
return AVPROBE_SCORE_EXTENSION / 2;
else
return 0;
}
static int loas_read_header(AVFormatContext *s)
{
AVStream *st;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = s->iformat->raw_codec_id;
st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
avpriv_set_pts_info(st, 64, 1, 28224000);
return 0;
}
FF_RAW_DEMUXER_CLASS(loas)
AVInputFormat ff_loas_demuxer = {
.name = "loas",
.long_name = NULL_IF_CONFIG_SMALL("LOAS AudioSyncStream"),
.read_probe = loas_probe,
.read_header = loas_read_header,
.read_packet = ff_raw_read_partial_packet,
.flags= AVFMT_GENERIC_INDEX,
.raw_codec_id = AV_CODEC_ID_AAC_LATM,
.priv_data_size = sizeof(FFRawDemuxerContext),
.priv_class = &loas_demuxer_class,
};
