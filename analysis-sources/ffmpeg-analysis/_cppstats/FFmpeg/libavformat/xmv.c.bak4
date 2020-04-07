


























#include <inttypes.h>

#include "libavutil/intreadwrite.h"

#include "avformat.h"
#include "internal.h"
#include "riff.h"
#include "libavutil/avassert.h"


#define XMV_MIN_HEADER_SIZE 36


#define XMV_AUDIO_ADPCM51_FRONTLEFTRIGHT 1

#define XMV_AUDIO_ADPCM51_FRONTCENTERLOW 2

#define XMV_AUDIO_ADPCM51_REARLEFTRIGHT 4


#define XMV_AUDIO_ADPCM51 (XMV_AUDIO_ADPCM51_FRONTLEFTRIGHT | XMV_AUDIO_ADPCM51_FRONTCENTERLOW | XMV_AUDIO_ADPCM51_REARLEFTRIGHT)



#define XMV_BLOCK_ALIGN_SIZE 36


typedef struct XMVVideoPacket {
int created;
int stream_index; 

uint32_t data_size; 
uint64_t data_offset; 

uint32_t current_frame; 
uint32_t frame_count; 

int has_extradata; 
uint8_t extradata[4]; 

int64_t last_pts; 
int64_t pts; 
} XMVVideoPacket;


typedef struct XMVAudioPacket {
int created;
int stream_index; 


uint16_t compression; 
uint16_t channels; 
int32_t sample_rate; 
uint16_t bits_per_sample; 
uint64_t bit_rate; 
uint16_t flags; 
unsigned block_align; 
uint16_t block_samples; 

enum AVCodecID codec_id; 

uint32_t data_size; 
uint64_t data_offset; 

uint32_t frame_size; 

uint64_t block_count; 
} XMVAudioPacket;


typedef struct XMVDemuxContext {
uint16_t audio_track_count; 

uint32_t this_packet_size; 
uint32_t next_packet_size; 

uint64_t this_packet_offset; 
uint64_t next_packet_offset; 

uint16_t current_stream; 
uint16_t stream_count; 

uint32_t video_duration;
uint32_t video_width;
uint32_t video_height;

XMVVideoPacket video; 
XMVAudioPacket *audio; 
} XMVDemuxContext;

static int xmv_probe(const AVProbeData *p)
{
uint32_t file_version;

if (p->buf_size < XMV_MIN_HEADER_SIZE)
return 0;

file_version = AV_RL32(p->buf + 16);
if ((file_version == 0) || (file_version > 4))
return 0;

if (!memcmp(p->buf + 12, "xobX", 4))
return AVPROBE_SCORE_MAX;

return 0;
}

static int xmv_read_close(AVFormatContext *s)
{
XMVDemuxContext *xmv = s->priv_data;

av_freep(&xmv->audio);

return 0;
}

static int xmv_read_header(AVFormatContext *s)
{
XMVDemuxContext *xmv = s->priv_data;
AVIOContext *pb = s->pb;

uint32_t file_version;
uint32_t this_packet_size;
uint16_t audio_track;
int ret;

s->ctx_flags |= AVFMTCTX_NOHEADER;

avio_skip(pb, 4); 

this_packet_size = avio_rl32(pb);

avio_skip(pb, 4); 
avio_skip(pb, 4); 

file_version = avio_rl32(pb);
if ((file_version != 4) && (file_version != 2))
avpriv_request_sample(s, "Uncommon version %"PRIu32"", file_version);



xmv->video_width = avio_rl32(pb);
xmv->video_height = avio_rl32(pb);
xmv->video_duration = avio_rl32(pb);



xmv->audio_track_count = avio_rl16(pb);

avio_skip(pb, 2); 

xmv->audio = av_mallocz_array(xmv->audio_track_count, sizeof(XMVAudioPacket));
if (!xmv->audio) {
ret = AVERROR(ENOMEM);
goto fail;
}

for (audio_track = 0; audio_track < xmv->audio_track_count; audio_track++) {
XMVAudioPacket *packet = &xmv->audio[audio_track];

packet->compression = avio_rl16(pb);
packet->channels = avio_rl16(pb);
packet->sample_rate = avio_rl32(pb);
packet->bits_per_sample = avio_rl16(pb);
packet->flags = avio_rl16(pb);

packet->bit_rate = (uint64_t)packet->bits_per_sample *
packet->sample_rate *
packet->channels;
packet->block_align = XMV_BLOCK_ALIGN_SIZE * packet->channels;
packet->block_samples = 64;
packet->codec_id = ff_wav_codec_get_id(packet->compression,
packet->bits_per_sample);

packet->stream_index = -1;

packet->frame_size = 0;
packet->block_count = 0;



if (packet->flags & XMV_AUDIO_ADPCM51)
av_log(s, AV_LOG_WARNING, "Unsupported 5.1 ADPCM audio stream "
"(0x%04X)\n", packet->flags);

if (!packet->channels || packet->sample_rate <= 0 ||
packet->channels >= UINT16_MAX / XMV_BLOCK_ALIGN_SIZE) {
av_log(s, AV_LOG_ERROR, "Invalid parameters for audio track %"PRIu16".\n",
audio_track);
ret = AVERROR_INVALIDDATA;
goto fail;
}
}




xmv->next_packet_offset = avio_tell(pb);
xmv->next_packet_size = this_packet_size - xmv->next_packet_offset;
xmv->stream_count = xmv->audio_track_count + 1;

return 0;

fail:
xmv_read_close(s);
return ret;
}

static void xmv_read_extradata(uint8_t *extradata, AVIOContext *pb)
{


uint32_t data = avio_rl32(pb);

int mspel_bit = !!(data & 0x01);
int loop_filter = !!(data & 0x02);
int abt_flag = !!(data & 0x04);
int j_type_bit = !!(data & 0x08);
int top_left_mv_flag = !!(data & 0x10);
int per_mb_rl_bit = !!(data & 0x20);
int slice_count = (data >> 6) & 7;



data = 0;

data |= mspel_bit << 15;
data |= loop_filter << 14;
data |= abt_flag << 13;
data |= j_type_bit << 12;
data |= top_left_mv_flag << 11;
data |= per_mb_rl_bit << 10;
data |= slice_count << 7;

AV_WB32(extradata, data);
}

static int xmv_process_packet_header(AVFormatContext *s)
{
XMVDemuxContext *xmv = s->priv_data;
AVIOContext *pb = s->pb;
int ret;

uint8_t data[8];
uint16_t audio_track;
uint64_t data_offset;


xmv->next_packet_size = avio_rl32(pb);



if (avio_read(pb, data, 8) != 8)
return AVERROR(EIO);

xmv->video.data_size = AV_RL32(data) & 0x007FFFFF;

xmv->video.current_frame = 0;
xmv->video.frame_count = (AV_RL32(data) >> 23) & 0xFF;

xmv->video.has_extradata = (data[3] & 0x80) != 0;

if (!xmv->video.created) {
AVStream *vst = avformat_new_stream(s, NULL);
if (!vst)
return AVERROR(ENOMEM);

avpriv_set_pts_info(vst, 32, 1, 1000);

vst->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
vst->codecpar->codec_id = AV_CODEC_ID_WMV2;
vst->codecpar->codec_tag = MKBETAG('W', 'M', 'V', '2');
vst->codecpar->width = xmv->video_width;
vst->codecpar->height = xmv->video_height;

vst->duration = xmv->video_duration;

xmv->video.stream_index = vst->index;

xmv->video.created = 1;
}










xmv->video.data_size -= xmv->audio_track_count * 4;

xmv->current_stream = 0;
if (!xmv->video.frame_count) {
xmv->video.frame_count = 1;
xmv->current_stream = xmv->stream_count > 1;
}



for (audio_track = 0; audio_track < xmv->audio_track_count; audio_track++) {
XMVAudioPacket *packet = &xmv->audio[audio_track];

if (avio_read(pb, data, 4) != 4)
return AVERROR(EIO);

if (!packet->created) {
AVStream *ast = avformat_new_stream(s, NULL);
if (!ast)
return AVERROR(ENOMEM);

ast->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
ast->codecpar->codec_id = packet->codec_id;
ast->codecpar->codec_tag = packet->compression;
ast->codecpar->channels = packet->channels;
ast->codecpar->sample_rate = packet->sample_rate;
ast->codecpar->bits_per_coded_sample = packet->bits_per_sample;
ast->codecpar->bit_rate = packet->bit_rate;
ast->codecpar->block_align = 36 * packet->channels;

avpriv_set_pts_info(ast, 32, packet->block_samples, packet->sample_rate);

packet->stream_index = ast->index;

ast->duration = xmv->video_duration;

packet->created = 1;
}

packet->data_size = AV_RL32(data) & 0x007FFFFF;
if ((packet->data_size == 0) && (audio_track != 0))





packet->data_size = xmv->audio[audio_track - 1].data_size;


packet->frame_size = packet->data_size / xmv->video.frame_count;
packet->frame_size -= packet->frame_size % packet->block_align;
}



data_offset = avio_tell(pb);

xmv->video.data_offset = data_offset;
data_offset += xmv->video.data_size;

for (audio_track = 0; audio_track < xmv->audio_track_count; audio_track++) {
xmv->audio[audio_track].data_offset = data_offset;
data_offset += xmv->audio[audio_track].data_size;
}




if (xmv->video.data_size > 0) {
if (xmv->video.has_extradata) {
xmv_read_extradata(xmv->video.extradata, pb);

xmv->video.data_size -= 4;
xmv->video.data_offset += 4;

if (xmv->video.stream_index >= 0) {
AVStream *vst = s->streams[xmv->video.stream_index];

av_assert0(xmv->video.stream_index < s->nb_streams);

if (vst->codecpar->extradata_size < 4) {
if ((ret = ff_alloc_extradata(vst->codecpar, 4)) < 0)
return ret;
}

memcpy(vst->codecpar->extradata, xmv->video.extradata, 4);
}
}
}

return 0;
}

static int xmv_fetch_new_packet(AVFormatContext *s)
{
XMVDemuxContext *xmv = s->priv_data;
AVIOContext *pb = s->pb;
int result;

if (xmv->this_packet_offset == xmv->next_packet_offset)
return AVERROR_EOF;


xmv->this_packet_offset = xmv->next_packet_offset;
if (avio_seek(pb, xmv->this_packet_offset, SEEK_SET) != xmv->this_packet_offset)
return AVERROR(EIO);


xmv->this_packet_size = xmv->next_packet_size;
if (xmv->this_packet_size < (12 + xmv->audio_track_count * 4))
return AVERROR(EIO);


result = xmv_process_packet_header(s);
if (result)
return result;


xmv->next_packet_offset = xmv->this_packet_offset + xmv->this_packet_size;

return 0;
}

static int xmv_fetch_audio_packet(AVFormatContext *s,
AVPacket *pkt, uint32_t stream)
{
XMVDemuxContext *xmv = s->priv_data;
AVIOContext *pb = s->pb;
XMVAudioPacket *audio = &xmv->audio[stream];

uint32_t data_size;
uint32_t block_count;
int result;


if (avio_seek(pb, audio->data_offset, SEEK_SET) != audio->data_offset)
return AVERROR(EIO);

if ((xmv->video.current_frame + 1) < xmv->video.frame_count)

data_size = FFMIN(audio->frame_size, audio->data_size);
else

data_size = audio->data_size;


result = av_get_packet(pb, pkt, data_size);
if (result <= 0)
return result;

pkt->stream_index = audio->stream_index;



block_count = data_size / audio->block_align;

pkt->duration = block_count;
pkt->pts = audio->block_count;
pkt->dts = AV_NOPTS_VALUE;

audio->block_count += block_count;


audio->data_size -= data_size;
audio->data_offset += data_size;

return 0;
}

static int xmv_fetch_video_packet(AVFormatContext *s,
AVPacket *pkt)
{
XMVDemuxContext *xmv = s->priv_data;
AVIOContext *pb = s->pb;
XMVVideoPacket *video = &xmv->video;

int result;
uint32_t frame_header;
uint32_t frame_size, frame_timestamp;
uint8_t *data, *end;


if (avio_seek(pb, video->data_offset, SEEK_SET) != video->data_offset)
return AVERROR(EIO);


frame_header = avio_rl32(pb);

frame_size = (frame_header & 0x1FFFF) * 4 + 4;
frame_timestamp = (frame_header >> 17);

if ((frame_size + 4) > video->data_size)
return AVERROR(EIO);


result = av_get_packet(pb, pkt, frame_size);
if (result != frame_size)
return result;





for (data = pkt->data, end = pkt->data + frame_size; data < end; data += 4)
AV_WB32(data, AV_RL32(data));

pkt->stream_index = video->stream_index;



video->last_pts = frame_timestamp + video->pts;

pkt->duration = 0;
pkt->pts = video->last_pts;
pkt->dts = AV_NOPTS_VALUE;

video->pts += frame_timestamp;


pkt->flags = (pkt->data[0] & 0x80) ? 0 : AV_PKT_FLAG_KEY;


video->data_size -= frame_size + 4;
video->data_offset += frame_size + 4;

return 0;
}

static int xmv_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
XMVDemuxContext *xmv = s->priv_data;
int result;

if (xmv->video.current_frame == xmv->video.frame_count) {


result = xmv_fetch_new_packet(s);
if (result)
return result;
}

if (xmv->current_stream == 0) {


result = xmv_fetch_video_packet(s, pkt);
} else {


result = xmv_fetch_audio_packet(s, pkt, xmv->current_stream - 1);
}
if (result) {
xmv->current_stream = 0;
xmv->video.current_frame = xmv->video.frame_count;
return result;
}



if (++xmv->current_stream >= xmv->stream_count) {
xmv->current_stream = 0;
xmv->video.current_frame += 1;
}

return 0;
}

AVInputFormat ff_xmv_demuxer = {
.name = "xmv",
.long_name = NULL_IF_CONFIG_SMALL("Microsoft XMV"),
.extensions = "xmv",
.priv_data_size = sizeof(XMVDemuxContext),
.read_probe = xmv_probe,
.read_header = xmv_read_header,
.read_packet = xmv_read_packet,
.read_close = xmv_read_close,
};
