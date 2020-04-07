#include <stdint.h>
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "avformat.h"
#include "internal.h"
#define EXTRADATA1_SIZE (6 + 256 * 3) 
#define FORM_TAG MKBETAG('F', 'O', 'R', 'M')
#define RLV2_TAG MKBETAG('R', 'L', 'V', '2')
#define RLV3_TAG MKBETAG('R', 'L', 'V', '3')
typedef struct Rl2DemuxContext {
unsigned int index_pos[2]; 
} Rl2DemuxContext;
static int rl2_probe(const AVProbeData *p)
{
if(AV_RB32(&p->buf[0]) != FORM_TAG)
return 0;
if(AV_RB32(&p->buf[8]) != RLV2_TAG &&
AV_RB32(&p->buf[8]) != RLV3_TAG)
return 0;
return AVPROBE_SCORE_MAX;
}
static av_cold int rl2_read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
AVStream *st;
unsigned int frame_count;
unsigned int audio_frame_counter = 0;
unsigned int video_frame_counter = 0;
unsigned int back_size;
unsigned short sound_rate;
unsigned short rate;
unsigned short channels;
unsigned short def_sound_size;
unsigned int signature;
unsigned int pts_den = 11025; 
unsigned int pts_num = 1103;
unsigned int* chunk_offset = NULL;
int* chunk_size = NULL;
int* audio_size = NULL;
int i;
int ret = 0;
avio_skip(pb,4); 
back_size = avio_rl32(pb); 
signature = avio_rb32(pb);
avio_skip(pb, 4); 
frame_count = avio_rl32(pb);
if(back_size > INT_MAX/2 || frame_count > INT_MAX / sizeof(uint32_t))
return AVERROR_INVALIDDATA;
avio_skip(pb, 2); 
sound_rate = avio_rl16(pb);
rate = avio_rl16(pb);
channels = avio_rl16(pb);
def_sound_size = avio_rl16(pb);
st = avformat_new_stream(s, NULL);
if(!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_RL2;
st->codecpar->codec_tag = 0; 
st->codecpar->width = 320;
st->codecpar->height = 200;
st->codecpar->extradata_size = EXTRADATA1_SIZE;
if(signature == RLV3_TAG && back_size > 0)
st->codecpar->extradata_size += back_size;
ret = ff_get_extradata(s, st->codecpar, pb, st->codecpar->extradata_size);
if (ret < 0)
return ret;
if(sound_rate){
if (!channels || channels > 42) {
av_log(s, AV_LOG_ERROR, "Invalid number of channels: %d\n", channels);
return AVERROR_INVALIDDATA;
}
pts_num = def_sound_size;
pts_den = rate;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
st->codecpar->codec_tag = 1;
st->codecpar->channels = channels;
st->codecpar->bits_per_coded_sample = 8;
st->codecpar->sample_rate = rate;
st->codecpar->bit_rate = st->codecpar->channels * st->codecpar->sample_rate *
st->codecpar->bits_per_coded_sample;
st->codecpar->block_align = st->codecpar->channels *
st->codecpar->bits_per_coded_sample / 8;
avpriv_set_pts_info(st,32,1,rate);
}
avpriv_set_pts_info(s->streams[0], 32, pts_num, pts_den);
chunk_size = av_malloc(frame_count * sizeof(uint32_t));
audio_size = av_malloc(frame_count * sizeof(uint32_t));
chunk_offset = av_malloc(frame_count * sizeof(uint32_t));
if(!chunk_size || !audio_size || !chunk_offset){
av_free(chunk_size);
av_free(audio_size);
av_free(chunk_offset);
return AVERROR(ENOMEM);
}
for(i=0; i < frame_count;i++) {
if (avio_feof(pb)) {
ret = AVERROR_INVALIDDATA;
goto end;
}
chunk_size[i] = avio_rl32(pb);
}
for(i=0; i < frame_count;i++) {
if (avio_feof(pb)) {
ret = AVERROR_INVALIDDATA;
goto end;
}
chunk_offset[i] = avio_rl32(pb);
}
for(i=0; i < frame_count;i++) {
if (avio_feof(pb)) {
ret = AVERROR_INVALIDDATA;
goto end;
}
audio_size[i] = avio_rl32(pb) & 0xFFFF;
}
for(i=0;i<frame_count;i++){
if(chunk_size[i] < 0 || audio_size[i] > chunk_size[i]){
ret = AVERROR_INVALIDDATA;
break;
}
if(sound_rate && audio_size[i]){
av_add_index_entry(s->streams[1], chunk_offset[i],
audio_frame_counter,audio_size[i], 0, AVINDEX_KEYFRAME);
audio_frame_counter += audio_size[i] / channels;
}
av_add_index_entry(s->streams[0], chunk_offset[i] + audio_size[i],
video_frame_counter,chunk_size[i]-audio_size[i],0,AVINDEX_KEYFRAME);
++video_frame_counter;
}
end:
av_free(chunk_size);
av_free(audio_size);
av_free(chunk_offset);
return ret;
}
static int rl2_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
Rl2DemuxContext *rl2 = s->priv_data;
AVIOContext *pb = s->pb;
AVIndexEntry *sample = NULL;
int i;
int ret = 0;
int stream_id = -1;
int64_t pos = INT64_MAX;
for(i=0; i<s->nb_streams; i++){
if(rl2->index_pos[i] < s->streams[i]->nb_index_entries
&& s->streams[i]->index_entries[ rl2->index_pos[i] ].pos < pos){
sample = &s->streams[i]->index_entries[ rl2->index_pos[i] ];
pos= sample->pos;
stream_id= i;
}
}
if(stream_id == -1)
return AVERROR_EOF;
++rl2->index_pos[stream_id];
avio_seek(pb, sample->pos, SEEK_SET);
ret = av_get_packet(pb, pkt, sample->size);
if(ret != sample->size){
return AVERROR(EIO);
}
pkt->stream_index = stream_id;
pkt->pts = sample->timestamp;
return ret;
}
static int rl2_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
AVStream *st = s->streams[stream_index];
Rl2DemuxContext *rl2 = s->priv_data;
int i;
int index = av_index_search_timestamp(st, timestamp, flags);
if(index < 0)
return -1;
rl2->index_pos[stream_index] = index;
timestamp = st->index_entries[index].timestamp;
for(i=0; i < s->nb_streams; i++){
AVStream *st2 = s->streams[i];
index = av_index_search_timestamp(st2,
av_rescale_q(timestamp, st->time_base, st2->time_base),
flags | AVSEEK_FLAG_BACKWARD);
if(index < 0)
index = 0;
rl2->index_pos[i] = index;
}
return 0;
}
AVInputFormat ff_rl2_demuxer = {
.name = "rl2",
.long_name = NULL_IF_CONFIG_SMALL("RL2"),
.priv_data_size = sizeof(Rl2DemuxContext),
.read_probe = rl2_probe,
.read_header = rl2_read_header,
.read_packet = rl2_read_packet,
.read_seek = rl2_read_seek,
};
