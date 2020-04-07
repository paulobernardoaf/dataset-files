






















#include "libavutil/attributes.h"
#include "libavutil/mathematics.h"
#include "avformat.h"
#include "internal.h"
#include "libavutil/dict.h"
#include "libavutil/intreadwrite.h"




#define NSV_MAX_RESYNC (500*1024)
#define NSV_MAX_RESYNC_TRIES 300
















































#if 0
struct NSVf_header {
uint32_t chunk_tag; 
uint32_t chunk_size;
uint32_t file_size; 
uint32_t file_length; 
uint32_t info_strings_size; 
uint32_t table_entries;
uint32_t table_entries_used; 
};

struct NSVs_header {
uint32_t chunk_tag; 
uint32_t v4cc; 
uint32_t a4cc; 
uint16_t vwidth; 
uint16_t vheight; 
uint8_t framerate; 
uint16_t unknown;
};

struct nsv_avchunk_header {
uint8_t vchunk_size_lsb;
uint16_t vchunk_size_msb; 
uint16_t achunk_size;
};

struct nsv_pcm_header {
uint8_t bits_per_sample;
uint8_t channel_count;
uint16_t sample_rate;
};
#endif









#define T_NSVF MKTAG('N', 'S', 'V', 'f') 
#define T_NSVS MKTAG('N', 'S', 'V', 's') 
#define T_TOC2 MKTAG('T', 'O', 'C', '2') 
#define T_NONE MKTAG('N', 'O', 'N', 'E') 
#define T_SUBT MKTAG('S', 'U', 'B', 'T') 
#define T_ASYN MKTAG('A', 'S', 'Y', 'N') 
#define T_KEYF MKTAG('K', 'E', 'Y', 'F') 

#define TB_NSVF MKBETAG('N', 'S', 'V', 'f')
#define TB_NSVS MKBETAG('N', 'S', 'V', 's')


#define NSV_ST_VIDEO 0
#define NSV_ST_AUDIO 1
#define NSV_ST_SUBT 2

enum NSVStatus {
NSV_UNSYNC,
NSV_FOUND_NSVF,
NSV_HAS_READ_NSVF,
NSV_FOUND_NSVS,
NSV_HAS_READ_NSVS,
NSV_FOUND_BEEF,
NSV_GOT_VIDEO,
NSV_GOT_AUDIO,
};

typedef struct NSVStream {
int frame_offset; 

int scale;
int rate;
int sample_size; 
int start;

int new_frame_offset; 
int cum_len; 
} NSVStream;

typedef struct NSVContext {
int base_offset;
int NSVf_end;
uint32_t *nsvs_file_offset;
int index_entries;
enum NSVStatus state;
AVPacket ahead[2]; 

int64_t duration;
uint32_t vtag, atag;
uint16_t vwidth, vheight;
int16_t avsync;
AVRational framerate;
uint32_t *nsvs_timestamps;
int nsvf;
} NSVContext;

static const AVCodecTag nsv_codec_video_tags[] = {
{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', ' ') },
{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', '0') },
{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', '1') },
{ AV_CODEC_ID_VP5, MKTAG('V', 'P', '5', ' ') },
{ AV_CODEC_ID_VP5, MKTAG('V', 'P', '5', '0') },
{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', ' ') },
{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '0') },
{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '1') },
{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '2') },
{ AV_CODEC_ID_VP8, MKTAG('V', 'P', '8', '0') },




{ AV_CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'D') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'G', 'B', '3') },
{ AV_CODEC_ID_NONE, 0 },
};

static const AVCodecTag nsv_codec_audio_tags[] = {
{ AV_CODEC_ID_MP3, MKTAG('M', 'P', '3', ' ') },
{ AV_CODEC_ID_AAC, MKTAG('A', 'A', 'C', ' ') },
{ AV_CODEC_ID_AAC, MKTAG('A', 'A', 'C', 'P') },
{ AV_CODEC_ID_AAC, MKTAG('V', 'L', 'B', ' ') },
{ AV_CODEC_ID_SPEEX, MKTAG('S', 'P', 'X', ' ') },
{ AV_CODEC_ID_PCM_U16LE, MKTAG('P', 'C', 'M', ' ') },
{ AV_CODEC_ID_NONE, 0 },
};


static int nsv_read_chunk(AVFormatContext *s, int fill_header);
static int nsv_read_close(AVFormatContext *s);


static int nsv_resync(AVFormatContext *s)
{
NSVContext *nsv = s->priv_data;
AVIOContext *pb = s->pb;
uint32_t v = 0;
int i;

for (i = 0; i < NSV_MAX_RESYNC; i++) {
if (avio_feof(pb)) {
av_log(s, AV_LOG_TRACE, "NSV EOF\n");
nsv->state = NSV_UNSYNC;
return -1;
}
v <<= 8;
v |= avio_r8(pb);
if (i < 8) {
av_log(s, AV_LOG_TRACE, "NSV resync: [%d] = %02"PRIx32"\n", i, v & 0x0FF);
}

if ((v & 0x0000ffff) == 0xefbe) { 
av_log(s, AV_LOG_TRACE, "NSV resynced on BEEF after %d bytes\n", i+1);
nsv->state = NSV_FOUND_BEEF;
return 0;
}

if (v == TB_NSVF) { 
av_log(s, AV_LOG_TRACE, "NSV resynced on NSVf after %d bytes\n", i+1);
nsv->state = NSV_FOUND_NSVF;
return 0;
}
if (v == MKBETAG('N', 'S', 'V', 's')) { 
av_log(s, AV_LOG_TRACE, "NSV resynced on NSVs after %d bytes\n", i+1);
nsv->state = NSV_FOUND_NSVS;
return 0;
}

}
av_log(s, AV_LOG_TRACE, "NSV sync lost\n");
return -1;
}

static int nsv_parse_NSVf_header(AVFormatContext *s)
{
NSVContext *nsv = s->priv_data;
AVIOContext *pb = s->pb;
unsigned int av_unused file_size;
unsigned int size;
int64_t duration;
int strings_size;
int table_entries;
int table_entries_used;

nsv->state = NSV_UNSYNC; 

if (nsv->nsvf) {
av_log(s, AV_LOG_TRACE, "Multiple NSVf\n");
return 0;
}
nsv->nsvf = 1;

size = avio_rl32(pb);
if (size < 28)
return -1;
nsv->NSVf_end = size;

file_size = (uint32_t)avio_rl32(pb);
av_log(s, AV_LOG_TRACE, "NSV NSVf chunk_size %u\n", size);
av_log(s, AV_LOG_TRACE, "NSV NSVf file_size %u\n", file_size);

nsv->duration = duration = avio_rl32(pb); 
av_log(s, AV_LOG_TRACE, "NSV NSVf duration %"PRId64" ms\n", duration);


strings_size = avio_rl32(pb);
table_entries = avio_rl32(pb);
table_entries_used = avio_rl32(pb);
av_log(s, AV_LOG_TRACE, "NSV NSVf info-strings size: %d, table entries: %d, bis %d\n",
strings_size, table_entries, table_entries_used);
if (avio_feof(pb))
return -1;

av_log(s, AV_LOG_TRACE, "NSV got header; filepos %"PRId64"\n", avio_tell(pb));

if (strings_size > 0) {
char *strings; 
char *p, *endp;
char *token, *value;
char quote;

p = strings = av_mallocz((size_t)strings_size + 1);
if (!p)
return AVERROR(ENOMEM);
endp = strings + strings_size;
avio_read(pb, strings, strings_size);
while (p < endp) {
while (*p == ' ')
p++; 
if (p >= endp-2)
break;
token = p;
p = strchr(p, '=');
if (!p || p >= endp-2)
break;
*p++ = '\0';
quote = *p++;
value = p;
p = strchr(p, quote);
if (!p || p >= endp)
break;
*p++ = '\0';
av_log(s, AV_LOG_TRACE, "NSV NSVf INFO: %s='%s'\n", token, value);
av_dict_set(&s->metadata, token, value, 0);
}
av_free(strings);
}
if (avio_feof(pb))
return -1;

av_log(s, AV_LOG_TRACE, "NSV got infos; filepos %"PRId64"\n", avio_tell(pb));

if (table_entries_used > 0) {
int i;
nsv->index_entries = table_entries_used;
if((unsigned)table_entries_used >= UINT_MAX / sizeof(uint32_t))
return -1;
nsv->nsvs_file_offset = av_malloc_array((unsigned)table_entries_used, sizeof(uint32_t));
if (!nsv->nsvs_file_offset)
return AVERROR(ENOMEM);

for(i=0;i<table_entries_used;i++) {
if (avio_feof(pb))
return AVERROR_INVALIDDATA;
nsv->nsvs_file_offset[i] = avio_rl32(pb) + size;
}

if(table_entries > table_entries_used &&
avio_rl32(pb) == MKTAG('T','O','C','2')) {
nsv->nsvs_timestamps = av_malloc_array((unsigned)table_entries_used, sizeof(uint32_t));
if (!nsv->nsvs_timestamps)
return AVERROR(ENOMEM);
for(i=0;i<table_entries_used;i++) {
nsv->nsvs_timestamps[i] = avio_rl32(pb);
}
}
}

av_log(s, AV_LOG_TRACE, "NSV got index; filepos %"PRId64"\n", avio_tell(pb));

avio_seek(pb, nsv->base_offset + size, SEEK_SET); 

if (avio_feof(pb))
return -1;
nsv->state = NSV_HAS_READ_NSVF;
return 0;
}

static int nsv_parse_NSVs_header(AVFormatContext *s)
{
NSVContext *nsv = s->priv_data;
AVIOContext *pb = s->pb;
uint32_t vtag, atag;
uint16_t vwidth, vheight;
AVRational framerate;
int i;
AVStream *st;
NSVStream *nst;

vtag = avio_rl32(pb);
atag = avio_rl32(pb);
vwidth = avio_rl16(pb);
vheight = avio_rl16(pb);
i = avio_r8(pb);

av_log(s, AV_LOG_TRACE, "NSV NSVs framerate code %2x\n", i);
if(i&0x80) { 
int t=(i & 0x7F)>>2;
if(t<16) framerate = (AVRational){1, t+1};
else framerate = (AVRational){t-15, 1};

if(i&1){
framerate.num *= 1000;
framerate.den *= 1001;
}

if((i&3)==3) framerate.num *= 24;
else if((i&3)==2) framerate.num *= 25;
else framerate.num *= 30;
}
else
framerate= (AVRational){i, 1};

nsv->avsync = avio_rl16(pb);
nsv->framerate = framerate;

av_log(s, AV_LOG_TRACE, "NSV NSVs vsize %dx%d\n", vwidth, vheight);


if (s->nb_streams == 0) { 
nsv->vtag = vtag;
nsv->atag = atag;
nsv->vwidth = vwidth;
nsv->vheight = vwidth;
if (vtag != T_NONE) {
int i;
st = avformat_new_stream(s, NULL);
if (!st)
goto fail;

st->id = NSV_ST_VIDEO;
nst = av_mallocz(sizeof(NSVStream));
if (!nst)
goto fail;
st->priv_data = nst;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_tag = vtag;
st->codecpar->codec_id = ff_codec_get_id(nsv_codec_video_tags, vtag);
st->codecpar->width = vwidth;
st->codecpar->height = vheight;
st->codecpar->bits_per_coded_sample = 24; 

avpriv_set_pts_info(st, 64, framerate.den, framerate.num);
st->start_time = 0;
st->duration = av_rescale(nsv->duration, framerate.num, 1000*framerate.den);

for(i=0;i<nsv->index_entries;i++) {
if(nsv->nsvs_timestamps) {
av_add_index_entry(st, nsv->nsvs_file_offset[i], nsv->nsvs_timestamps[i],
0, 0, AVINDEX_KEYFRAME);
} else {
int64_t ts = av_rescale(i*nsv->duration/nsv->index_entries, framerate.num, 1000*framerate.den);
av_add_index_entry(st, nsv->nsvs_file_offset[i], ts, 0, 0, AVINDEX_KEYFRAME);
}
}
}
if (atag != T_NONE) {
st = avformat_new_stream(s, NULL);
if (!st)
goto fail;

st->id = NSV_ST_AUDIO;
nst = av_mallocz(sizeof(NSVStream));
if (!nst)
goto fail;
st->priv_data = nst;
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_tag = atag;
st->codecpar->codec_id = ff_codec_get_id(nsv_codec_audio_tags, atag);

st->need_parsing = AVSTREAM_PARSE_FULL; 


avpriv_set_pts_info(st, 64, 1, framerate.num*1000);
st->start_time = 0;
st->duration = (int64_t)nsv->duration * framerate.num;
}
} else {
if (nsv->vtag != vtag || nsv->atag != atag || nsv->vwidth != vwidth || nsv->vheight != vwidth) {
av_log(s, AV_LOG_TRACE, "NSV NSVs header values differ from the first one!!!\n");

}
}

nsv->state = NSV_HAS_READ_NSVS;
return 0;
fail:

nsv->state = NSV_UNSYNC;
return -1;
}

static int nsv_read_header(AVFormatContext *s)
{
NSVContext *nsv = s->priv_data;
int i, err;

nsv->state = NSV_UNSYNC;
nsv->ahead[0].data = nsv->ahead[1].data = NULL;

for (i = 0; i < NSV_MAX_RESYNC_TRIES; i++) {
err = nsv_resync(s);
if (err < 0)
goto fail;
if (nsv->state == NSV_FOUND_NSVF) {
err = nsv_parse_NSVf_header(s);
if (err < 0)
goto fail;
}

if (nsv->state == NSV_FOUND_NSVS) {
err = nsv_parse_NSVs_header(s);
if (err < 0)
goto fail;
break; 
}
}
if (s->nb_streams < 1) { 
err = AVERROR_INVALIDDATA;
goto fail;
}


err = nsv_read_chunk(s, 1);
fail:
if (err < 0)
nsv_read_close(s);

av_log(s, AV_LOG_TRACE, "parsed header\n");
return err;
}

static int nsv_read_chunk(AVFormatContext *s, int fill_header)
{
NSVContext *nsv = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *st[2] = {NULL, NULL};
NSVStream *nst;
AVPacket *pkt;
int i, err = 0;
uint8_t auxcount; 
uint32_t vsize;
uint16_t asize;
uint16_t auxsize;
int ret;

if (nsv->ahead[0].data || nsv->ahead[1].data)
return 0; 

null_chunk_retry:
if (avio_feof(pb))
return -1;

for (i = 0; i < NSV_MAX_RESYNC_TRIES && nsv->state < NSV_FOUND_NSVS && !err; i++)
err = nsv_resync(s);
if (err < 0)
return err;
if (nsv->state == NSV_FOUND_NSVS)
err = nsv_parse_NSVs_header(s);
if (err < 0)
return err;
if (nsv->state != NSV_HAS_READ_NSVS && nsv->state != NSV_FOUND_BEEF)
return -1;

auxcount = avio_r8(pb);
vsize = avio_rl16(pb);
asize = avio_rl16(pb);
vsize = (vsize << 4) | (auxcount >> 4);
auxcount &= 0x0f;
av_log(s, AV_LOG_TRACE, "NSV CHUNK %d aux, %"PRIu32" bytes video, %d bytes audio\n", auxcount, vsize, asize);

for (i = 0; i < auxcount; i++) {
uint32_t av_unused auxtag;
auxsize = avio_rl16(pb);
auxtag = avio_rl32(pb);
avio_skip(pb, auxsize);
vsize -= auxsize + sizeof(uint16_t) + sizeof(uint32_t); 
}

if (avio_feof(pb))
return -1;
if (!vsize && !asize) {
nsv->state = NSV_UNSYNC;
goto null_chunk_retry;
}


if (s->nb_streams > 0)
st[s->streams[0]->id] = s->streams[0];
if (s->nb_streams > 1)
st[s->streams[1]->id] = s->streams[1];

if (vsize && st[NSV_ST_VIDEO]) {
nst = st[NSV_ST_VIDEO]->priv_data;
pkt = &nsv->ahead[NSV_ST_VIDEO];
if ((ret = av_get_packet(pb, pkt, vsize)) < 0)
return ret;
pkt->stream_index = st[NSV_ST_VIDEO]->index;
pkt->dts = nst->frame_offset;
pkt->flags |= nsv->state == NSV_HAS_READ_NSVS ? AV_PKT_FLAG_KEY : 0; 
for (i = 0; i < FFMIN(8, vsize); i++)
av_log(s, AV_LOG_TRACE, "NSV video: [%d] = %02x\n", i, pkt->data[i]);
}
if(st[NSV_ST_VIDEO])
((NSVStream*)st[NSV_ST_VIDEO]->priv_data)->frame_offset++;

if (asize && st[NSV_ST_AUDIO]) {
nst = st[NSV_ST_AUDIO]->priv_data;
pkt = &nsv->ahead[NSV_ST_AUDIO];


if (asize && st[NSV_ST_AUDIO]->codecpar->codec_tag == MKTAG('P', 'C', 'M', ' ')) {
uint8_t bps;
uint8_t channels;
uint16_t samplerate;
bps = avio_r8(pb);
channels = avio_r8(pb);
samplerate = avio_rl16(pb);
if (!channels || !samplerate)
return AVERROR_INVALIDDATA;
asize-=4;
av_log(s, AV_LOG_TRACE, "NSV RAWAUDIO: bps %d, nchan %d, srate %d\n", bps, channels, samplerate);
if (fill_header) {
st[NSV_ST_AUDIO]->need_parsing = AVSTREAM_PARSE_NONE; 
if (bps != 16) {
av_log(s, AV_LOG_TRACE, "NSV AUDIO bit/sample != 16 (%d)!!!\n", bps);
}
bps /= channels; 
if (bps == 8)
st[NSV_ST_AUDIO]->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
samplerate /= 4;
channels = 1;
st[NSV_ST_AUDIO]->codecpar->channels = channels;
st[NSV_ST_AUDIO]->codecpar->sample_rate = samplerate;
av_log(s, AV_LOG_TRACE, "NSV RAWAUDIO: bps %d, nchan %d, srate %d\n", bps, channels, samplerate);
}
}
if ((ret = av_get_packet(pb, pkt, asize)) < 0)
return ret;
pkt->stream_index = st[NSV_ST_AUDIO]->index;
pkt->flags |= nsv->state == NSV_HAS_READ_NSVS ? AV_PKT_FLAG_KEY : 0; 
if( nsv->state == NSV_HAS_READ_NSVS && st[NSV_ST_VIDEO] ) {

pkt->dts = (((NSVStream*)st[NSV_ST_VIDEO]->priv_data)->frame_offset-1);
pkt->dts *= (int64_t)1000 * nsv->framerate.den;
pkt->dts += (int64_t)nsv->avsync * nsv->framerate.num;
av_log(s, AV_LOG_TRACE, "NSV AUDIO: sync:%d, dts:%"PRId64, nsv->avsync, pkt->dts);
}
nst->frame_offset++;
}

nsv->state = NSV_UNSYNC;
return 0;
}


static int nsv_read_packet(AVFormatContext *s, AVPacket *pkt)
{
NSVContext *nsv = s->priv_data;
int i, err = 0;


if (!nsv->ahead[0].data && !nsv->ahead[1].data)
err = nsv_read_chunk(s, 0);
if (err < 0)
return err;


for (i = 0; i < 2; i++) {
if (nsv->ahead[i].data) {
av_packet_move_ref(pkt, &nsv->ahead[i]);
return 0;
}
}


return -1;
}

static int nsv_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
NSVContext *nsv = s->priv_data;
AVStream *st = s->streams[stream_index];
NSVStream *nst = st->priv_data;
int index;

index = av_index_search_timestamp(st, timestamp, flags);
if(index < 0)
return -1;

if (avio_seek(s->pb, st->index_entries[index].pos, SEEK_SET) < 0)
return -1;

nst->frame_offset = st->index_entries[index].timestamp;
nsv->state = NSV_UNSYNC;
return 0;
}

static int nsv_read_close(AVFormatContext *s)
{
NSVContext *nsv = s->priv_data;

av_freep(&nsv->nsvs_file_offset);
av_freep(&nsv->nsvs_timestamps);
if (nsv->ahead[0].data)
av_packet_unref(&nsv->ahead[0]);
if (nsv->ahead[1].data)
av_packet_unref(&nsv->ahead[1]);
return 0;
}

static int nsv_probe(const AVProbeData *p)
{
int i, score = 0;



if (p->buf[0] == 'N' && p->buf[1] == 'S' &&
p->buf[2] == 'V' && (p->buf[3] == 'f' || p->buf[3] == 's'))
return AVPROBE_SCORE_MAX;




for (i = 1; i < p->buf_size - 3; i++) {
if (AV_RL32(p->buf + i) == AV_RL32("NSVs")) {

int vsize = AV_RL24(p->buf+i+19) >> 4;
int asize = AV_RL16(p->buf+i+22);
int offset = i + 23 + asize + vsize + 1;
if (offset <= p->buf_size - 2 && AV_RL16(p->buf + offset) == 0xBEEF)
return 4*AVPROBE_SCORE_MAX/5;
score = AVPROBE_SCORE_MAX/5;
}
}

if (av_match_ext(p->filename, "nsv"))
return AVPROBE_SCORE_EXTENSION;

return score;
}

AVInputFormat ff_nsv_demuxer = {
.name = "nsv",
.long_name = NULL_IF_CONFIG_SMALL("Nullsoft Streaming Video"),
.priv_data_size = sizeof(NSVContext),
.read_probe = nsv_probe,
.read_header = nsv_read_header,
.read_packet = nsv_read_packet,
.read_close = nsv_read_close,
.read_seek = nsv_read_seek,
};
