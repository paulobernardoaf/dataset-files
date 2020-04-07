




















#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "avformat.h"
#include "internal.h"
#include "lrc.h"
#include "metadata.h"
#include "subtitles.h"
#include "version.h"
#include "libavutil/bprint.h"
#include "libavutil/dict.h"
#include "libavutil/log.h"
#include "libavutil/macros.h"

static int lrc_write_header(AVFormatContext *s)
{
const AVDictionaryEntry *metadata_item;

if(s->nb_streams != 1 ||
s->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
av_log(s, AV_LOG_ERROR,
"LRC supports only a single subtitle stream.\n");
return AVERROR(EINVAL);
}
if(s->streams[0]->codecpar->codec_id != AV_CODEC_ID_SUBRIP &&
s->streams[0]->codecpar->codec_id != AV_CODEC_ID_TEXT) {
av_log(s, AV_LOG_ERROR, "Unsupported subtitle codec: %s\n",
avcodec_get_name(s->streams[0]->codecpar->codec_id));
return AVERROR(EINVAL);
}
avpriv_set_pts_info(s->streams[0], 64, 1, 100);

ff_standardize_creation_time(s);
ff_metadata_conv_ctx(s, ff_lrc_metadata_conv, NULL);
if(!(s->flags & AVFMT_FLAG_BITEXACT)) { 




av_dict_set(&s->metadata, "ve", AV_STRINGIFY(LIBAVFORMAT_VERSION), 0);
} else {
av_dict_set(&s->metadata, "ve", NULL, 0);
}
for(metadata_item = NULL;
(metadata_item = av_dict_get(s->metadata, "", metadata_item,
AV_DICT_IGNORE_SUFFIX));) {
char *delim;
if(!metadata_item->value[0]) {
continue;
}
while((delim = strchr(metadata_item->value, '\n'))) {
*delim = ' ';
}
while((delim = strchr(metadata_item->value, '\r'))) {
*delim = ' ';
}
avio_printf(s->pb, "[%s:%s]\n",
metadata_item->key, metadata_item->value);
}
avio_printf(s->pb, "\n");
return 0;
}

static int lrc_write_packet(AVFormatContext *s, AVPacket *pkt)
{
if(pkt->pts != AV_NOPTS_VALUE) {
char *data = av_malloc(pkt->size + 1);
char *line;
char *delim;

if(!data) {
return AVERROR(ENOMEM);
}
memcpy(data, pkt->data, pkt->size);
data[pkt->size] = '\0';

for(delim = data + pkt->size - 1;
delim >= data && (delim[0] == '\n' || delim[0] == '\r'); delim--) {
delim[0] = '\0'; 
}
line = data;
while(line[0] == '\n' || line[0] == '\r') {
line++; 
}

while(line) {
delim = strchr(line, '\n');
if(delim) {
if(delim > line && delim[-1] == '\r') {
delim[-1] = '\0';
}
delim[0] = '\0';
delim++;
}
if(line[0] == '[') {
av_log(s, AV_LOG_WARNING,
"Subtitle starts with '[', may cause problems with LRC format.\n");
}

if(pkt->pts >= 0) {
avio_printf(s->pb, "[%02"PRId64":%02"PRId64".%02"PRId64"]",
(pkt->pts / 6000),
((pkt->pts / 100) % 60),
(pkt->pts % 100));
} else {


avio_printf(s->pb, "[-%02"PRId64":%02"PRId64".%02"PRId64"]",
(-pkt->pts) / 6000,
((-pkt->pts) / 100) % 60,
(-pkt->pts) % 100);
}
avio_printf(s->pb, "%s\n", line);
line = delim;
}
av_free(data);
}
return 0;
}

AVOutputFormat ff_lrc_muxer = {
.name = "lrc",
.long_name = NULL_IF_CONFIG_SMALL("LRC lyrics"),
.extensions = "lrc",
.priv_data_size = 0,
.write_header = lrc_write_header,
.write_packet = lrc_write_packet,
.flags = AVFMT_VARIABLE_FPS | AVFMT_GLOBALHEADER |
AVFMT_TS_NEGATIVE | AVFMT_TS_NONSTRICT,
.subtitle_codec = AV_CODEC_ID_SUBRIP
};
