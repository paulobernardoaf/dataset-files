





















#include "libavcodec/put_bits.h"
#include "libavutil/avassert.h"
#include "avformat.h"
#include "swf.h"

static void put_swf_tag(AVFormatContext *s, int tag)
{
SWFContext *swf = s->priv_data;
AVIOContext *pb = s->pb;

swf->tag_pos = avio_tell(pb);
swf->tag = tag;

if (tag & TAG_LONG) {
avio_wl16(pb, 0);
avio_wl32(pb, 0);
} else {
avio_wl16(pb, 0);
}
}

static void put_swf_end_tag(AVFormatContext *s)
{
SWFContext *swf = s->priv_data;
AVIOContext *pb = s->pb;
int64_t pos;
int tag_len, tag;

pos = avio_tell(pb);
tag_len = pos - swf->tag_pos - 2;
tag = swf->tag;
avio_seek(pb, swf->tag_pos, SEEK_SET);
if (tag & TAG_LONG) {
tag &= ~TAG_LONG;
avio_wl16(pb, (tag << 6) | 0x3f);
avio_wl32(pb, tag_len - 4);
} else {
av_assert0(tag_len < 0x3f);
avio_wl16(pb, (tag << 6) | tag_len);
}
avio_seek(pb, pos, SEEK_SET);
}

static inline void max_nbits(int *nbits_ptr, int val)
{
int n;

if (val == 0)
return;
val = FFABS(val);
n = 1;
while (val != 0) {
n++;
val >>= 1;
}
if (n > *nbits_ptr)
*nbits_ptr = n;
}

static void put_swf_rect(AVIOContext *pb,
int xmin, int xmax, int ymin, int ymax)
{
PutBitContext p;
uint8_t buf[256];
int nbits, mask;

init_put_bits(&p, buf, sizeof(buf));

nbits = 0;
max_nbits(&nbits, xmin);
max_nbits(&nbits, xmax);
max_nbits(&nbits, ymin);
max_nbits(&nbits, ymax);
mask = (1 << nbits) - 1;


put_bits(&p, 5, nbits);
put_bits(&p, nbits, xmin & mask);
put_bits(&p, nbits, xmax & mask);
put_bits(&p, nbits, ymin & mask);
put_bits(&p, nbits, ymax & mask);

flush_put_bits(&p);
avio_write(pb, buf, put_bits_ptr(&p) - p.buf);
}

static void put_swf_line_edge(PutBitContext *pb, int dx, int dy)
{
int nbits, mask;

put_bits(pb, 1, 1); 
put_bits(pb, 1, 1); 
nbits = 2;
max_nbits(&nbits, dx);
max_nbits(&nbits, dy);

mask = (1 << nbits) - 1;
put_bits(pb, 4, nbits - 2); 
if (dx == 0) {
put_bits(pb, 1, 0);
put_bits(pb, 1, 1);
put_bits(pb, nbits, dy & mask);
} else if (dy == 0) {
put_bits(pb, 1, 0);
put_bits(pb, 1, 0);
put_bits(pb, nbits, dx & mask);
} else {
put_bits(pb, 1, 1);
put_bits(pb, nbits, dx & mask);
put_bits(pb, nbits, dy & mask);
}
}

#define FRAC_BITS 16

static void put_swf_matrix(AVIOContext *pb,
int a, int b, int c, int d, int tx, int ty)
{
PutBitContext p;
uint8_t buf[256];
int nbits;

init_put_bits(&p, buf, sizeof(buf));

put_bits(&p, 1, 1); 
nbits = 1;
max_nbits(&nbits, a);
max_nbits(&nbits, d);
put_bits(&p, 5, nbits); 
put_bits(&p, nbits, a);
put_bits(&p, nbits, d);

put_bits(&p, 1, 1); 
nbits = 1;
max_nbits(&nbits, c);
max_nbits(&nbits, b);
put_bits(&p, 5, nbits); 
put_bits(&p, nbits, c);
put_bits(&p, nbits, b);

nbits = 1;
max_nbits(&nbits, tx);
max_nbits(&nbits, ty);
put_bits(&p, 5, nbits); 
put_bits(&p, nbits, tx);
put_bits(&p, nbits, ty);

flush_put_bits(&p);
avio_write(pb, buf, put_bits_ptr(&p) - p.buf);
}

static int swf_write_header(AVFormatContext *s)
{
SWFContext *swf = s->priv_data;
AVIOContext *pb = s->pb;
PutBitContext p;
uint8_t buf1[256];
int i, width, height, rate, rate_base;
int version;

swf->sound_samples = 0;
swf->swf_frame_number = 0;
swf->video_frame_number = 0;

for(i=0;i<s->nb_streams;i++) {
AVCodecParameters *par = s->streams[i]->codecpar;
if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
if (swf->audio_par) {
av_log(s, AV_LOG_ERROR, "SWF muxer only supports 1 audio stream\n");
return AVERROR_INVALIDDATA;
}
if (par->codec_id == AV_CODEC_ID_MP3) {
swf->audio_par = par;
swf->audio_fifo= av_fifo_alloc(AUDIO_FIFO_SIZE);
if (!swf->audio_fifo)
return AVERROR(ENOMEM);
} else {
av_log(s, AV_LOG_ERROR, "SWF muxer only supports MP3\n");
return -1;
}
} else {
if (swf->video_par) {
av_log(s, AV_LOG_ERROR, "SWF muxer only supports 1 video stream\n");
return AVERROR_INVALIDDATA;
}
if (par->codec_id == AV_CODEC_ID_VP6F ||
par->codec_id == AV_CODEC_ID_FLV1 ||
par->codec_id == AV_CODEC_ID_MJPEG) {
swf->video_st = s->streams[i];
swf->video_par = par;
} else {
av_log(s, AV_LOG_ERROR, "SWF muxer only supports VP6, FLV1 and MJPEG\n");
return -1;
}
}
}

if (!swf->video_par) {

width = 320;
height = 200;
rate = 10;
rate_base= 1;
} else {
width = swf->video_par->width;
height = swf->video_par->height;

rate = swf->video_st->time_base.den;
rate_base = swf->video_st->time_base.num;
}

if (!swf->audio_par)
swf->samples_per_frame = (44100LL * rate_base) / rate;
else
swf->samples_per_frame = (swf->audio_par->sample_rate * rate_base) / rate;

avio_write(pb, "FWS", 3);

if (!strcmp("avm2", s->oformat->name))
version = 9;
else if (swf->video_par && swf->video_par->codec_id == AV_CODEC_ID_VP6F)
version = 8; 
else if (swf->video_par && swf->video_par->codec_id == AV_CODEC_ID_FLV1)
version = 6; 
else
version = 4; 
avio_w8(pb, version);

avio_wl32(pb, DUMMY_FILE_SIZE); 


put_swf_rect(pb, 0, width * 20, 0, height * 20);
if ((rate * 256LL) / rate_base >= (1<<16)) {
av_log(s, AV_LOG_ERROR, "Invalid (too large) frame rate %d/%d\n", rate, rate_base);
return AVERROR(EINVAL);
}
avio_wl16(pb, (rate * 256) / rate_base); 
swf->duration_pos = avio_tell(pb);
avio_wl16(pb, (uint16_t)(DUMMY_DURATION * (int64_t)rate / rate_base)); 


if (version == 9) {
put_swf_tag(s, TAG_FILEATTRIBUTES);
avio_wl32(pb, 1<<3); 
put_swf_end_tag(s);
}


if (swf->video_par && swf->video_par->codec_id == AV_CODEC_ID_MJPEG) {
put_swf_tag(s, TAG_DEFINESHAPE);

avio_wl16(pb, SHAPE_ID); 

put_swf_rect(pb, 0, width, 0, height);

avio_w8(pb, 1); 
avio_w8(pb, 0x41); 
avio_wl16(pb, BITMAP_ID); 

put_swf_matrix(pb, 1 << FRAC_BITS, 0,
0, 1 << FRAC_BITS, 0, 0);
avio_w8(pb, 0); 


init_put_bits(&p, buf1, sizeof(buf1));
put_bits(&p, 4, 1); 
put_bits(&p, 4, 0); 

put_bits(&p, 1, 0); 
put_bits(&p, 5, FLAG_MOVETO | FLAG_SETFILL0);
put_bits(&p, 5, 1); 
put_bits(&p, 1, 0); 
put_bits(&p, 1, 0); 
put_bits(&p, 1, 1); 


put_swf_line_edge(&p, width, 0);
put_swf_line_edge(&p, 0, height);
put_swf_line_edge(&p, -width, 0);
put_swf_line_edge(&p, 0, -height);


put_bits(&p, 1, 0); 
put_bits(&p, 5, 0);

flush_put_bits(&p);
avio_write(pb, buf1, put_bits_ptr(&p) - p.buf);

put_swf_end_tag(s);
}

if (swf->audio_par && swf->audio_par->codec_id == AV_CODEC_ID_MP3) {
int v = 0;


put_swf_tag(s, TAG_STREAMHEAD2);
switch(swf->audio_par->sample_rate) {
case 11025: v |= 1 << 2; break;
case 22050: v |= 2 << 2; break;
case 44100: v |= 3 << 2; break;
default:

av_log(s, AV_LOG_ERROR, "swf does not support that sample rate, choose from (44100, 22050, 11025).\n");
return -1;
}
v |= 0x02; 
if (swf->audio_par->channels == 2)
v |= 0x01; 
avio_w8(s->pb, v);
v |= 0x20; 
avio_w8(s->pb, v);
avio_wl16(s->pb, swf->samples_per_frame); 
avio_wl16(s->pb, 0);

put_swf_end_tag(s);
}

return 0;
}

static int swf_write_video(AVFormatContext *s,
AVCodecParameters *par, const uint8_t *buf, int size)
{
SWFContext *swf = s->priv_data;
AVIOContext *pb = s->pb;


if (swf->swf_frame_number == 16000)
av_log(s, AV_LOG_INFO, "warning: Flash Player limit of 16000 frames reached\n");

if (par->codec_id == AV_CODEC_ID_VP6F ||
par->codec_id == AV_CODEC_ID_FLV1) {
if (swf->video_frame_number == 0) {

put_swf_tag(s, TAG_VIDEOSTREAM);
avio_wl16(pb, VIDEO_ID);
swf->vframes_pos = avio_tell(pb);
avio_wl16(pb, 15000); 
avio_wl16(pb, par->width);
avio_wl16(pb, par->height);
avio_w8(pb, 0);
avio_w8(pb,ff_codec_get_tag(ff_swf_codec_tags, par->codec_id));
put_swf_end_tag(s);


put_swf_tag(s, TAG_PLACEOBJECT2);
avio_w8(pb, 0x36);
avio_wl16(pb, 1);
avio_wl16(pb, VIDEO_ID);
put_swf_matrix(pb, 1 << FRAC_BITS, 0, 0, 1 << FRAC_BITS, 0, 0);
avio_wl16(pb, swf->video_frame_number);
avio_write(pb, "video", 5);
avio_w8(pb, 0x00);
put_swf_end_tag(s);
} else {

put_swf_tag(s, TAG_PLACEOBJECT2);
avio_w8(pb, 0x11);
avio_wl16(pb, 1);
avio_wl16(pb, swf->video_frame_number);
put_swf_end_tag(s);
}


put_swf_tag(s, TAG_VIDEOFRAME | TAG_LONG);
avio_wl16(pb, VIDEO_ID);
avio_wl16(pb, swf->video_frame_number++);
avio_write(pb, buf, size);
put_swf_end_tag(s);
} else if (par->codec_id == AV_CODEC_ID_MJPEG) {
if (swf->swf_frame_number > 0) {

put_swf_tag(s, TAG_REMOVEOBJECT);
avio_wl16(pb, SHAPE_ID); 
avio_wl16(pb, 1); 
put_swf_end_tag(s);


put_swf_tag(s, TAG_FREECHARACTER);
avio_wl16(pb, BITMAP_ID);
put_swf_end_tag(s);
}

put_swf_tag(s, TAG_JPEG2 | TAG_LONG);

avio_wl16(pb, BITMAP_ID); 


avio_wb32(pb, 0xffd8ffd9);

avio_write(pb, buf, size);

put_swf_end_tag(s);



put_swf_tag(s, TAG_PLACEOBJECT);
avio_wl16(pb, SHAPE_ID); 
avio_wl16(pb, 1); 
put_swf_matrix(pb, 20 << FRAC_BITS, 0, 0, 20 << FRAC_BITS, 0, 0);
put_swf_end_tag(s);
}

swf->swf_frame_number++;


if (swf->audio_par && av_fifo_size(swf->audio_fifo)) {
int frame_size = av_fifo_size(swf->audio_fifo);
put_swf_tag(s, TAG_STREAMBLOCK | TAG_LONG);
avio_wl16(pb, swf->sound_samples);
avio_wl16(pb, 0); 
av_fifo_generic_read(swf->audio_fifo, pb, frame_size, (void*)avio_write);
put_swf_end_tag(s);


swf->sound_samples = 0;
}


put_swf_tag(s, TAG_SHOWFRAME);
put_swf_end_tag(s);

return 0;
}

static int swf_write_audio(AVFormatContext *s,
AVCodecParameters *par, uint8_t *buf, int size)
{
SWFContext *swf = s->priv_data;


if (swf->swf_frame_number == 16000)
av_log(s, AV_LOG_INFO, "warning: Flash Player limit of 16000 frames reached\n");

if (av_fifo_size(swf->audio_fifo) + size > AUDIO_FIFO_SIZE) {
av_log(s, AV_LOG_ERROR, "audio fifo too small to mux audio essence\n");
return -1;
}

av_fifo_generic_write(swf->audio_fifo, buf, size, NULL);
swf->sound_samples += av_get_audio_frame_duration2(par, size);


if (!swf->video_par)
swf_write_video(s, par, 0, 0);

return 0;
}

static int swf_write_packet(AVFormatContext *s, AVPacket *pkt)
{
AVCodecParameters *par = s->streams[pkt->stream_index]->codecpar;
if (par->codec_type == AVMEDIA_TYPE_AUDIO)
return swf_write_audio(s, par, pkt->data, pkt->size);
else
return swf_write_video(s, par, pkt->data, pkt->size);
}

static int swf_write_trailer(AVFormatContext *s)
{
SWFContext *swf = s->priv_data;
AVIOContext *pb = s->pb;
AVCodecParameters *par, *video_par;
int file_size, i;

video_par = NULL;
for(i=0;i<s->nb_streams;i++) {
par = s->streams[i]->codecpar;
if (par->codec_type == AVMEDIA_TYPE_VIDEO)
video_par = par;
else {
av_fifo_freep(&swf->audio_fifo);
}
}

put_swf_tag(s, TAG_END);
put_swf_end_tag(s);


if ((s->pb->seekable & AVIO_SEEKABLE_NORMAL) && video_par) {
file_size = avio_tell(pb);
avio_seek(pb, 4, SEEK_SET);
avio_wl32(pb, file_size);
avio_seek(pb, swf->duration_pos, SEEK_SET);
avio_wl16(pb, swf->video_frame_number);
if (swf->vframes_pos) {
avio_seek(pb, swf->vframes_pos, SEEK_SET);
avio_wl16(pb, swf->video_frame_number);
}
avio_seek(pb, file_size, SEEK_SET);
}
return 0;
}

#if CONFIG_SWF_MUXER
AVOutputFormat ff_swf_muxer = {
.name = "swf",
.long_name = NULL_IF_CONFIG_SMALL("SWF (ShockWave Flash)"),
.mime_type = "application/x-shockwave-flash",
.extensions = "swf",
.priv_data_size = sizeof(SWFContext),
.audio_codec = AV_CODEC_ID_MP3,
.video_codec = AV_CODEC_ID_FLV1,
.write_header = swf_write_header,
.write_packet = swf_write_packet,
.write_trailer = swf_write_trailer,
.flags = AVFMT_TS_NONSTRICT,
};
#endif
#if CONFIG_AVM2_MUXER
AVOutputFormat ff_avm2_muxer = {
.name = "avm2",
.long_name = NULL_IF_CONFIG_SMALL("SWF (ShockWave Flash) (AVM2)"),
.mime_type = "application/x-shockwave-flash",
.priv_data_size = sizeof(SWFContext),
.audio_codec = AV_CODEC_ID_MP3,
.video_codec = AV_CODEC_ID_FLV1,
.write_header = swf_write_header,
.write_packet = swf_write_packet,
.write_trailer = swf_write_trailer,
.flags = AVFMT_TS_NONSTRICT,
};
#endif
