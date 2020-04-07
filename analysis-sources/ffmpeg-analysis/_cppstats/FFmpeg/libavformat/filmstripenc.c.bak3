

























#include "libavutil/intreadwrite.h"
#include "avformat.h"

#define RAND_TAG MKBETAG('R','a','n','d')

typedef struct FilmstripMuxContext {
int nb_frames;
} FilmstripMuxContext;

static int write_header(AVFormatContext *s)
{
if (s->streams[0]->codecpar->format != AV_PIX_FMT_RGBA) {
av_log(s, AV_LOG_ERROR, "only AV_PIX_FMT_RGBA is supported\n");
return AVERROR_INVALIDDATA;
}
return 0;
}

static int write_packet(AVFormatContext *s, AVPacket *pkt)
{
FilmstripMuxContext *film = s->priv_data;
avio_write(s->pb, pkt->data, pkt->size);
film->nb_frames++;
return 0;
}

static int write_trailer(AVFormatContext *s)
{
FilmstripMuxContext *film = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *st = s->streams[0];
int i;

avio_wb32(pb, RAND_TAG);
avio_wb32(pb, film->nb_frames);
avio_wb16(pb, 0); 
avio_wb16(pb, 0); 
avio_wb16(pb, st->codecpar->width);
avio_wb16(pb, st->codecpar->height);
avio_wb16(pb, 0); 

avio_wb16(pb, st->time_base.den / st->time_base.num);
for (i = 0; i < 16; i++)
avio_w8(pb, 0x00); 

return 0;
}

AVOutputFormat ff_filmstrip_muxer = {
.name = "filmstrip",
.long_name = NULL_IF_CONFIG_SMALL("Adobe Filmstrip"),
.extensions = "flm",
.priv_data_size = sizeof(FilmstripMuxContext),
.audio_codec = AV_CODEC_ID_NONE,
.video_codec = AV_CODEC_ID_RAWVIDEO,
.write_header = write_header,
.write_packet = write_packet,
.write_trailer = write_trailer,
};
