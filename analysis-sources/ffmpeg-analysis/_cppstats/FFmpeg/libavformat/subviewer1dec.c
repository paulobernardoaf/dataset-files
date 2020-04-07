#include "avformat.h"
#include "internal.h"
#include "subtitles.h"
typedef struct {
FFDemuxSubtitlesQueue q;
} SubViewer1Context;
static int subviewer1_probe(const AVProbeData *p)
{
const unsigned char *ptr = p->buf;
if (strstr(ptr, "******** START SCRIPT ********"))
return AVPROBE_SCORE_EXTENSION;
return 0;
}
static int subviewer1_read_header(AVFormatContext *s)
{
int delay = 0;
AVPacket *sub = NULL;
SubViewer1Context *subviewer1 = s->priv_data;
AVStream *st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avpriv_set_pts_info(st, 64, 1, 1);
st->codecpar->codec_type = AVMEDIA_TYPE_SUBTITLE;
st->codecpar->codec_id = AV_CODEC_ID_SUBVIEWER1;
while (!avio_feof(s->pb)) {
char line[4096];
int len = ff_get_line(s->pb, line, sizeof(line));
int hh, mm, ss;
if (!len)
break;
if (!strncmp(line, "[DELAY]", 7)) {
ff_get_line(s->pb, line, sizeof(line));
sscanf(line, "%d", &delay);
}
if (sscanf(line, "[%d:%d:%d]", &hh, &mm, &ss) == 3) {
const int64_t pos = avio_tell(s->pb);
int64_t pts_start = hh*3600LL + mm*60LL + ss + delay;
len = ff_get_line(s->pb, line, sizeof(line));
line[strcspn(line, "\r\n")] = 0;
if (!*line) {
if (sub)
sub->duration = pts_start - sub->pts;
} else {
sub = ff_subtitles_queue_insert(&subviewer1->q, line, len, 0);
if (!sub)
return AVERROR(ENOMEM);
sub->pos = pos;
sub->pts = pts_start;
sub->duration = -1;
}
}
}
ff_subtitles_queue_finalize(s, &subviewer1->q);
return 0;
}
static int subviewer1_read_packet(AVFormatContext *s, AVPacket *pkt)
{
SubViewer1Context *subviewer1 = s->priv_data;
return ff_subtitles_queue_read_packet(&subviewer1->q, pkt);
}
static int subviewer1_read_seek(AVFormatContext *s, int stream_index,
int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
{
SubViewer1Context *subviewer1 = s->priv_data;
return ff_subtitles_queue_seek(&subviewer1->q, s, stream_index,
min_ts, ts, max_ts, flags);
}
static int subviewer1_read_close(AVFormatContext *s)
{
SubViewer1Context *subviewer1 = s->priv_data;
ff_subtitles_queue_clean(&subviewer1->q);
return 0;
}
AVInputFormat ff_subviewer1_demuxer = {
.name = "subviewer1",
.long_name = NULL_IF_CONFIG_SMALL("SubViewer v1 subtitle format"),
.priv_data_size = sizeof(SubViewer1Context),
.read_probe = subviewer1_probe,
.read_header = subviewer1_read_header,
.read_packet = subviewer1_read_packet,
.read_seek2 = subviewer1_read_seek,
.read_close = subviewer1_read_close,
.extensions = "sub",
};
