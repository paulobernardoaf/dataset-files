




























#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"

#define VMD_HEADER_SIZE 0x0330
#define BYTES_PER_FRAME_RECORD 16

typedef struct vmd_frame {
int stream_index;
int64_t frame_offset;
unsigned int frame_size;
int64_t pts;
int keyframe;
unsigned char frame_record[BYTES_PER_FRAME_RECORD];
} vmd_frame;

typedef struct VmdDemuxContext {
int video_stream_index;
int audio_stream_index;

unsigned int frame_count;
unsigned int frames_per_block;
vmd_frame *frame_table;
unsigned int current_frame;
int is_indeo3;

int sample_rate;
int64_t audio_sample_counter;
int skiphdr;

unsigned char vmd_header[VMD_HEADER_SIZE];
} VmdDemuxContext;

static int vmd_probe(const AVProbeData *p)
{
int w, h, sample_rate;
if (p->buf_size < 806)
return 0;


if (AV_RL16(&p->buf[0]) != VMD_HEADER_SIZE - 2)
return 0;
w = AV_RL16(&p->buf[12]);
h = AV_RL16(&p->buf[14]);
sample_rate = AV_RL16(&p->buf[804]);
if ((!w || w > 2048 || !h || h > 2048) &&
sample_rate != 22050)
return 0;


return AVPROBE_SCORE_EXTENSION;
}

static int vmd_read_header(AVFormatContext *s)
{
VmdDemuxContext *vmd = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *st = NULL, *vst = NULL;
unsigned int toc_offset;
unsigned char *raw_frame_table;
int raw_frame_table_size;
int64_t current_offset;
int i, j, ret;
int width, height;
unsigned int total_frames;
int64_t current_audio_pts = 0;
unsigned char chunk[BYTES_PER_FRAME_RECORD];
int num, den;
int sound_buffers;


avio_seek(pb, 0, SEEK_SET);
if (avio_read(pb, vmd->vmd_header, VMD_HEADER_SIZE) != VMD_HEADER_SIZE)
return AVERROR(EIO);

width = AV_RL16(&vmd->vmd_header[12]);
height = AV_RL16(&vmd->vmd_header[14]);
if (width && height) {
if(vmd->vmd_header[24] == 'i' && vmd->vmd_header[25] == 'v' && vmd->vmd_header[26] == '3') {
vmd->is_indeo3 = 1;
} else {
vmd->is_indeo3 = 0;
}

vst = avformat_new_stream(s, NULL);
if (!vst)
return AVERROR(ENOMEM);
avpriv_set_pts_info(vst, 33, 1, 10);
vmd->video_stream_index = vst->index;
vst->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
vst->codecpar->codec_id = vmd->is_indeo3 ? AV_CODEC_ID_INDEO3 : AV_CODEC_ID_VMDVIDEO;
vst->codecpar->codec_tag = 0; 
vst->codecpar->width = width;
vst->codecpar->height = height;
if(vmd->is_indeo3 && vst->codecpar->width > 320){
vst->codecpar->width >>= 1;
vst->codecpar->height >>= 1;
}
if ((ret = ff_alloc_extradata(vst->codecpar, VMD_HEADER_SIZE)) < 0)
return ret;
memcpy(vst->codecpar->extradata, vmd->vmd_header, VMD_HEADER_SIZE);
}


vmd->sample_rate = AV_RL16(&vmd->vmd_header[804]);
if (vmd->sample_rate) {
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
vmd->audio_stream_index = st->index;
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_VMDAUDIO;
st->codecpar->codec_tag = 0; 
st->codecpar->sample_rate = vmd->sample_rate;
st->codecpar->block_align = AV_RL16(&vmd->vmd_header[806]);
if (st->codecpar->block_align & 0x8000) {
st->codecpar->bits_per_coded_sample = 16;
st->codecpar->block_align = -(st->codecpar->block_align - 0x10000);
} else {
st->codecpar->bits_per_coded_sample = 8;
}
if (vmd->vmd_header[811] & 0x80) {
st->codecpar->channels = 2;
st->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
} else if (vmd->vmd_header[811] & 0x2) {


st->codecpar->channels = 2;
st->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
st->codecpar->block_align = st->codecpar->block_align << 1;
} else {
st->codecpar->channels = 1;
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
}
st->codecpar->bit_rate = st->codecpar->sample_rate *
st->codecpar->bits_per_coded_sample * st->codecpar->channels;


num = st->codecpar->block_align;
den = st->codecpar->sample_rate * st->codecpar->channels;
av_reduce(&num, &den, num, den, (1UL<<31)-1);
if (vst)
avpriv_set_pts_info(vst, 33, num, den);
avpriv_set_pts_info(st, 33, num, den);
}

toc_offset = AV_RL32(&vmd->vmd_header[812]);
vmd->frame_count = AV_RL16(&vmd->vmd_header[6]);
vmd->frames_per_block = AV_RL16(&vmd->vmd_header[18]);
avio_seek(pb, toc_offset, SEEK_SET);

raw_frame_table = NULL;
vmd->frame_table = NULL;
sound_buffers = AV_RL16(&vmd->vmd_header[808]);
raw_frame_table_size = vmd->frame_count * 6;
if(vmd->frame_count * vmd->frames_per_block >= UINT_MAX / sizeof(vmd_frame) - sound_buffers){
av_log(s, AV_LOG_ERROR, "vmd->frame_count * vmd->frames_per_block too large\n");
return -1;
}
raw_frame_table = av_malloc(raw_frame_table_size);
vmd->frame_table = av_malloc_array(vmd->frame_count * vmd->frames_per_block + sound_buffers, sizeof(vmd_frame));
if (!raw_frame_table || !vmd->frame_table) {
ret = AVERROR(ENOMEM);
goto error;
}
if (avio_read(pb, raw_frame_table, raw_frame_table_size) !=
raw_frame_table_size) {
ret = AVERROR(EIO);
goto error;
}

total_frames = 0;
for (i = 0; i < vmd->frame_count; i++) {

current_offset = AV_RL32(&raw_frame_table[6 * i + 2]);


for (j = 0; j < vmd->frames_per_block; j++) {
int type;
uint32_t size;

if ((ret = avio_read(pb, chunk, BYTES_PER_FRAME_RECORD)) != BYTES_PER_FRAME_RECORD) {
av_log(s, AV_LOG_ERROR, "Failed to read frame record\n");
if (ret >= 0)
ret = AVERROR_INVALIDDATA;
goto error;
}
type = chunk[0];
size = AV_RL32(&chunk[2]);
if (size > INT_MAX / 2) {
av_log(s, AV_LOG_ERROR, "Invalid frame size\n");
ret = AVERROR_INVALIDDATA;
goto error;
}
if(!size && type != 1)
continue;
switch(type) {
case 1: 
if (!st) break;

vmd->frame_table[total_frames].frame_offset = current_offset;
vmd->frame_table[total_frames].stream_index = vmd->audio_stream_index;
vmd->frame_table[total_frames].frame_size = size;
memcpy(vmd->frame_table[total_frames].frame_record, chunk, BYTES_PER_FRAME_RECORD);
vmd->frame_table[total_frames].pts = current_audio_pts;
total_frames++;
if(!current_audio_pts)
current_audio_pts += sound_buffers - 1;
else
current_audio_pts++;
break;
case 2: 
vmd->frame_table[total_frames].frame_offset = current_offset;
vmd->frame_table[total_frames].stream_index = vmd->video_stream_index;
vmd->frame_table[total_frames].frame_size = size;
memcpy(vmd->frame_table[total_frames].frame_record, chunk, BYTES_PER_FRAME_RECORD);
vmd->frame_table[total_frames].pts = i;
total_frames++;
break;
}
current_offset += size;
}
}

av_free(raw_frame_table);

vmd->current_frame = 0;
vmd->frame_count = total_frames;

return 0;

error:
av_freep(&raw_frame_table);
av_freep(&vmd->frame_table);
return ret;
}

static int vmd_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
VmdDemuxContext *vmd = s->priv_data;
AVIOContext *pb = s->pb;
int ret = 0;
vmd_frame *frame;

if (vmd->current_frame >= vmd->frame_count)
return AVERROR_EOF;

frame = &vmd->frame_table[vmd->current_frame];

avio_seek(pb, frame->frame_offset, SEEK_SET);

if(ffio_limit(pb, frame->frame_size) != frame->frame_size)
return AVERROR(EIO);
ret = av_new_packet(pkt, frame->frame_size + BYTES_PER_FRAME_RECORD);
if (ret < 0)
return ret;
pkt->pos= avio_tell(pb);
memcpy(pkt->data, frame->frame_record, BYTES_PER_FRAME_RECORD);
if(vmd->is_indeo3 && frame->frame_record[0] == 0x02)
ret = avio_read(pb, pkt->data, frame->frame_size);
else
ret = avio_read(pb, pkt->data + BYTES_PER_FRAME_RECORD,
frame->frame_size);

if (ret != frame->frame_size) {
ret = AVERROR(EIO);
}
pkt->stream_index = frame->stream_index;
pkt->pts = frame->pts;
av_log(s, AV_LOG_DEBUG, " dispatching %s frame with %d bytes and pts %"PRId64"\n",
(frame->frame_record[0] == 0x02) ? "video" : "audio",
frame->frame_size + BYTES_PER_FRAME_RECORD,
pkt->pts);

vmd->current_frame++;

return ret;
}

static int vmd_read_close(AVFormatContext *s)
{
VmdDemuxContext *vmd = s->priv_data;

av_freep(&vmd->frame_table);

return 0;
}

AVInputFormat ff_vmd_demuxer = {
.name = "vmd",
.long_name = NULL_IF_CONFIG_SMALL("Sierra VMD"),
.priv_data_size = sizeof(VmdDemuxContext),
.read_probe = vmd_probe,
.read_header = vmd_read_header,
.read_packet = vmd_read_packet,
.read_close = vmd_read_close,
};
