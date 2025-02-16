#include "libavutil/intreadwrite.h"
#include "libavutil/intfloat.h"
#include "libavutil/dict.h"
#include "avformat.h"
#include "avio_internal.h"
#include "rawenc.h"
#include "sox.h"
typedef struct SoXContext {
int64_t header_size;
} SoXContext;
static int sox_write_header(AVFormatContext *s)
{
SoXContext *sox = s->priv_data;
AVIOContext *pb = s->pb;
AVCodecParameters *par = s->streams[0]->codecpar;
AVDictionaryEntry *comment;
size_t comment_len = 0, comment_size;
comment = av_dict_get(s->metadata, "comment", NULL, 0);
if (comment)
comment_len = strlen(comment->value);
comment_size = FFALIGN(comment_len, 8);
sox->header_size = SOX_FIXED_HDR + comment_size;
if (par->codec_id == AV_CODEC_ID_PCM_S32LE) {
ffio_wfourcc(pb, ".SoX");
avio_wl32(pb, sox->header_size);
avio_wl64(pb, 0); 
avio_wl64(pb, av_double2int(par->sample_rate));
avio_wl32(pb, par->channels);
avio_wl32(pb, comment_size);
} else if (par->codec_id == AV_CODEC_ID_PCM_S32BE) {
ffio_wfourcc(pb, "XoS.");
avio_wb32(pb, sox->header_size);
avio_wb64(pb, 0); 
avio_wb64(pb, av_double2int(par->sample_rate));
avio_wb32(pb, par->channels);
avio_wb32(pb, comment_size);
} else {
av_log(s, AV_LOG_ERROR, "invalid codec; use pcm_s32le or pcm_s32be\n");
return AVERROR(EINVAL);
}
if (comment_len)
avio_write(pb, comment->value, comment_len);
ffio_fill(pb, 0, comment_size - comment_len);
return 0;
}
static int sox_write_trailer(AVFormatContext *s)
{
SoXContext *sox = s->priv_data;
AVIOContext *pb = s->pb;
AVCodecParameters *par = s->streams[0]->codecpar;
if (s->pb->seekable & AVIO_SEEKABLE_NORMAL) {
int64_t file_size = avio_tell(pb);
int64_t num_samples = (file_size - sox->header_size - 4LL) >> 2LL;
avio_seek(pb, 8, SEEK_SET);
if (par->codec_id == AV_CODEC_ID_PCM_S32LE) {
avio_wl64(pb, num_samples);
} else
avio_wb64(pb, num_samples);
avio_seek(pb, file_size, SEEK_SET);
}
return 0;
}
AVOutputFormat ff_sox_muxer = {
.name = "sox",
.long_name = NULL_IF_CONFIG_SMALL("SoX native"),
.extensions = "sox",
.priv_data_size = sizeof(SoXContext),
.audio_codec = AV_CODEC_ID_PCM_S32LE,
.video_codec = AV_CODEC_ID_NONE,
.write_header = sox_write_header,
.write_packet = ff_raw_write_packet,
.write_trailer = sox_write_trailer,
.flags = AVFMT_NOTIMESTAMPS,
};
