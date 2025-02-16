#include <inttypes.h>
#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/bswap.h"
#include "libavutil/common.h"
#include "libavutil/dict.h"
#include "libavutil/internal.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "avio_internal.h"
#include "avlanguage.h"
#include "id3v2.h"
#include "internal.h"
#include "riff.h"
#include "asf.h"
#include "asfcrypt.h"
typedef struct ASFPayload {
uint8_t type;
uint16_t size;
} ASFPayload;
typedef struct ASFStream {
int num;
unsigned char seq;
AVPacket pkt;
int frag_offset;
int packet_obj_size;
int timestamp;
int64_t duration;
int skip_to_key;
int pkt_clean;
int ds_span; 
int ds_packet_size;
int ds_chunk_size;
int64_t packet_pos;
uint16_t stream_language_index;
int palette_changed;
uint32_t palette[256];
int payload_ext_ct;
ASFPayload payload[8];
} ASFStream;
typedef struct ASFContext {
const AVClass *class;
int asfid2avid[128]; 
ASFStream streams[128]; 
uint32_t stream_bitrates[128]; 
AVRational dar[128];
char stream_languages[128][6]; 
int packet_size_left;
uint64_t data_offset; 
uint64_t data_object_offset; 
uint64_t data_object_size; 
int index_read;
ASFMainHeader hdr;
int packet_flags;
int packet_property;
int packet_timestamp;
int packet_segsizetype;
int packet_segments;
int packet_seq;
int packet_replic_size;
int packet_key_frame;
int packet_padsize;
unsigned int packet_frag_offset;
unsigned int packet_frag_size;
int64_t packet_frag_timestamp;
int ts_is_pts;
int packet_multi_size;
int packet_time_delta;
int packet_time_start;
int64_t packet_pos;
int stream_index;
ASFStream *asf_st; 
int no_resync_search;
int export_xmp;
int uses_std_ecc;
} ASFContext;
static const AVOption options[] = {
{ "no_resync_search", "Don't try to resynchronize by looking for a certain optional start code", offsetof(ASFContext, no_resync_search), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM },
{ "export_xmp", "Export full XMP metadata", offsetof(ASFContext, export_xmp), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM },
{ NULL },
};
static const AVClass asf_class = {
.class_name = "asf demuxer",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
#undef NDEBUG
#include <assert.h>
#define ASF_MAX_STREAMS 127
#define FRAME_HEADER_SIZE 6
#if defined(DEBUG)
static const ff_asf_guid stream_bitrate_guid = { 
0xce, 0x75, 0xf8, 0x7b, 0x8d, 0x46, 0xd1, 0x11, 0x8d, 0x82, 0x00, 0x60, 0x97, 0xc9, 0xa2, 0xb2
};
#define PRINT_IF_GUID(g, cmp) if (!ff_guidcmp(g, &cmp)) av_log(NULL, AV_LOG_TRACE, "(GUID: %s) ", #cmp)
static void print_guid(ff_asf_guid *g)
{
int i;
PRINT_IF_GUID(g, ff_asf_header);
else PRINT_IF_GUID(g, ff_asf_file_header);
else PRINT_IF_GUID(g, ff_asf_stream_header);
else PRINT_IF_GUID(g, ff_asf_audio_stream);
else PRINT_IF_GUID(g, ff_asf_audio_conceal_none);
else PRINT_IF_GUID(g, ff_asf_video_stream);
else PRINT_IF_GUID(g, ff_asf_video_conceal_none);
else PRINT_IF_GUID(g, ff_asf_command_stream);
else PRINT_IF_GUID(g, ff_asf_comment_header);
else PRINT_IF_GUID(g, ff_asf_codec_comment_header);
else PRINT_IF_GUID(g, ff_asf_codec_comment1_header);
else PRINT_IF_GUID(g, ff_asf_data_header);
else PRINT_IF_GUID(g, ff_asf_simple_index_header);
else PRINT_IF_GUID(g, ff_asf_head1_guid);
else PRINT_IF_GUID(g, ff_asf_head2_guid);
else PRINT_IF_GUID(g, ff_asf_my_guid);
else PRINT_IF_GUID(g, ff_asf_ext_stream_header);
else PRINT_IF_GUID(g, ff_asf_extended_content_header);
else PRINT_IF_GUID(g, ff_asf_ext_stream_embed_stream_header);
else PRINT_IF_GUID(g, ff_asf_ext_stream_audio_stream);
else PRINT_IF_GUID(g, ff_asf_metadata_header);
else PRINT_IF_GUID(g, ff_asf_metadata_library_header);
else PRINT_IF_GUID(g, ff_asf_marker_header);
else PRINT_IF_GUID(g, stream_bitrate_guid);
else PRINT_IF_GUID(g, ff_asf_language_guid);
else
av_log(NULL, AV_LOG_TRACE, "(GUID: unknown) ");
for (i = 0; i < 16; i++)
av_log(NULL, AV_LOG_TRACE, " 0x%02x,", (*g)[i]);
av_log(NULL, AV_LOG_TRACE, "}\n");
}
#undef PRINT_IF_GUID
#else
#define print_guid(g) while(0)
#endif
static int asf_probe(const AVProbeData *pd)
{
if (!ff_guidcmp(pd->buf, &ff_asf_header))
return AVPROBE_SCORE_MAX;
else
return 0;
}
static int get_value(AVIOContext *pb, int type, int type2_size)
{
switch (type) {
case 2:
return (type2_size == 32) ? avio_rl32(pb) : avio_rl16(pb);
case 3:
return avio_rl32(pb);
case 4:
return avio_rl64(pb);
case 5:
return avio_rl16(pb);
default:
return INT_MIN;
}
}
static int asf_read_picture(AVFormatContext *s, int len)
{
AVPacket pkt = { 0 };
const CodecMime *mime = ff_id3v2_mime_tags;
enum AVCodecID id = AV_CODEC_ID_NONE;
char mimetype[64];
uint8_t *desc = NULL;
AVStream *st = NULL;
int ret, type, picsize, desc_len;
if (len < 1 + 4 + 2 + 2) {
av_log(s, AV_LOG_ERROR, "Invalid attached picture size: %d.\n", len);
return AVERROR_INVALIDDATA;
}
type = avio_r8(s->pb);
len--;
if (type >= FF_ARRAY_ELEMS(ff_id3v2_picture_types) || type < 0) {
av_log(s, AV_LOG_WARNING, "Unknown attached picture type: %d.\n", type);
type = 0;
}
picsize = avio_rl32(s->pb);
len -= 4;
len -= avio_get_str16le(s->pb, len, mimetype, sizeof(mimetype));
while (mime->id != AV_CODEC_ID_NONE) {
if (!strncmp(mime->str, mimetype, sizeof(mimetype))) {
id = mime->id;
break;
}
mime++;
}
if (id == AV_CODEC_ID_NONE) {
av_log(s, AV_LOG_ERROR, "Unknown attached picture mimetype: %s.\n",
mimetype);
return 0;
}
if (picsize >= len) {
av_log(s, AV_LOG_ERROR, "Invalid attached picture data size: %d >= %d.\n",
picsize, len);
return AVERROR_INVALIDDATA;
}
desc_len = (len - picsize) * 2 + 1;
desc = av_malloc(desc_len);
if (!desc)
return AVERROR(ENOMEM);
len -= avio_get_str16le(s->pb, len - picsize, desc, desc_len);
ret = av_get_packet(s->pb, &pkt, picsize);
if (ret < 0)
goto fail;
st = avformat_new_stream(s, NULL);
if (!st) {
ret = AVERROR(ENOMEM);
goto fail;
}
st->disposition |= AV_DISPOSITION_ATTACHED_PIC;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = id;
st->attached_pic = pkt;
st->attached_pic.stream_index = st->index;
st->attached_pic.flags |= AV_PKT_FLAG_KEY;
if (*desc)
av_dict_set(&st->metadata, "title", desc, AV_DICT_DONT_STRDUP_VAL);
else
av_freep(&desc);
av_dict_set(&st->metadata, "comment", ff_id3v2_picture_types[type], 0);
return 0;
fail:
av_freep(&desc);
av_packet_unref(&pkt);
return ret;
}
static void get_id3_tag(AVFormatContext *s, int len)
{
ID3v2ExtraMeta *id3v2_extra_meta = NULL;
ff_id3v2_read(s, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta, len);
if (id3v2_extra_meta) {
ff_id3v2_parse_apic(s, &id3v2_extra_meta);
ff_id3v2_parse_chapters(s, &id3v2_extra_meta);
}
ff_id3v2_free_extra_meta(&id3v2_extra_meta);
}
static void get_tag(AVFormatContext *s, const char *key, int type, int len, int type2_size)
{
ASFContext *asf = s->priv_data;
char *value = NULL;
int64_t off = avio_tell(s->pb);
#define LEN 22
av_assert0((unsigned)len < (INT_MAX - LEN) / 2);
if (!asf->export_xmp && !strncmp(key, "xmp", 3))
goto finish;
value = av_malloc(2 * len + LEN);
if (!value)
goto finish;
switch (type) {
case ASF_UNICODE:
avio_get_str16le(s->pb, len, value, 2 * len + 1);
break;
case -1: 
avio_read(s->pb, value, len);
value[len]=0;
break;
case ASF_BYTE_ARRAY:
if (!strcmp(key, "WM/Picture")) { 
asf_read_picture(s, len);
} else if (!strcmp(key, "ID3")) { 
get_id3_tag(s, len);
} else {
av_log(s, AV_LOG_VERBOSE, "Unsupported byte array in tag %s.\n", key);
}
goto finish;
case ASF_BOOL:
case ASF_DWORD:
case ASF_QWORD:
case ASF_WORD: {
uint64_t num = get_value(s->pb, type, type2_size);
snprintf(value, LEN, "%"PRIu64, num);
break;
}
case ASF_GUID:
av_log(s, AV_LOG_DEBUG, "Unsupported GUID value in tag %s.\n", key);
goto finish;
default:
av_log(s, AV_LOG_DEBUG,
"Unsupported value type %d in tag %s.\n", type, key);
goto finish;
}
if (*value)
av_dict_set(&s->metadata, key, value, 0);
finish:
av_freep(&value);
avio_seek(s->pb, off + len, SEEK_SET);
}
static int asf_read_file_properties(AVFormatContext *s, int64_t size)
{
ASFContext *asf = s->priv_data;
AVIOContext *pb = s->pb;
ff_get_guid(pb, &asf->hdr.guid);
asf->hdr.file_size = avio_rl64(pb);
asf->hdr.create_time = avio_rl64(pb);
avio_rl64(pb); 
asf->hdr.play_time = avio_rl64(pb);
asf->hdr.send_time = avio_rl64(pb);
asf->hdr.preroll = avio_rl32(pb);
asf->hdr.ignore = avio_rl32(pb);
asf->hdr.flags = avio_rl32(pb);
asf->hdr.min_pktsize = avio_rl32(pb);
asf->hdr.max_pktsize = avio_rl32(pb);
if (asf->hdr.min_pktsize >= (1U << 29))
return AVERROR_INVALIDDATA;
asf->hdr.max_bitrate = avio_rl32(pb);
s->packet_size = asf->hdr.max_pktsize;
return 0;
}
static int asf_read_stream_properties(AVFormatContext *s, int64_t size)
{
ASFContext *asf = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *st;
ASFStream *asf_st;
ff_asf_guid g;
enum AVMediaType type;
int type_specific_size, sizeX;
unsigned int tag1;
int64_t pos1, pos2, start_time;
int test_for_ext_stream_audio, is_dvr_ms_audio = 0;
if (s->nb_streams == ASF_MAX_STREAMS) {
av_log(s, AV_LOG_ERROR, "too many streams\n");
return AVERROR(EINVAL);
}
pos1 = avio_tell(pb);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avpriv_set_pts_info(st, 32, 1, 1000); 
start_time = asf->hdr.preroll;
if (!(asf->hdr.flags & 0x01)) { 
int64_t fsize = avio_size(pb);
if (fsize <= 0 || (int64_t)asf->hdr.file_size <= 0 ||
20*FFABS(fsize - (int64_t)asf->hdr.file_size) < FFMIN(fsize, asf->hdr.file_size))
st->duration = asf->hdr.play_time /
(10000000 / 1000) - start_time;
}
ff_get_guid(pb, &g);
test_for_ext_stream_audio = 0;
if (!ff_guidcmp(&g, &ff_asf_audio_stream)) {
type = AVMEDIA_TYPE_AUDIO;
} else if (!ff_guidcmp(&g, &ff_asf_video_stream)) {
type = AVMEDIA_TYPE_VIDEO;
} else if (!ff_guidcmp(&g, &ff_asf_jfif_media)) {
type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_MJPEG;
} else if (!ff_guidcmp(&g, &ff_asf_command_stream)) {
type = AVMEDIA_TYPE_DATA;
} else if (!ff_guidcmp(&g, &ff_asf_ext_stream_embed_stream_header)) {
test_for_ext_stream_audio = 1;
type = AVMEDIA_TYPE_UNKNOWN;
} else {
return -1;
}
ff_get_guid(pb, &g);
avio_skip(pb, 8); 
type_specific_size = avio_rl32(pb);
avio_rl32(pb);
st->id = avio_rl16(pb) & 0x7f; 
asf->asfid2avid[st->id] = s->nb_streams - 1;
asf_st = &asf->streams[st->id];
avio_rl32(pb);
if (test_for_ext_stream_audio) {
ff_get_guid(pb, &g);
if (!ff_guidcmp(&g, &ff_asf_ext_stream_audio_stream)) {
type = AVMEDIA_TYPE_AUDIO;
is_dvr_ms_audio = 1;
ff_get_guid(pb, &g);
avio_rl32(pb);
avio_rl32(pb);
avio_rl32(pb);
ff_get_guid(pb, &g);
avio_rl32(pb);
}
}
st->codecpar->codec_type = type;
if (type == AVMEDIA_TYPE_AUDIO) {
int ret = ff_get_wav_header(s, pb, st->codecpar, type_specific_size, 0);
if (ret < 0)
return ret;
if (is_dvr_ms_audio) {
st->request_probe = 1;
st->codecpar->codec_tag = 0;
}
if (st->codecpar->codec_id == AV_CODEC_ID_AAC)
st->need_parsing = AVSTREAM_PARSE_NONE;
else
st->need_parsing = AVSTREAM_PARSE_FULL;
pos2 = avio_tell(pb);
if (size >= (pos2 + 8 - pos1 + 24)) {
asf_st->ds_span = avio_r8(pb);
asf_st->ds_packet_size = avio_rl16(pb);
asf_st->ds_chunk_size = avio_rl16(pb);
avio_rl16(pb); 
avio_r8(pb); 
}
if (asf_st->ds_span > 1) {
if (!asf_st->ds_chunk_size ||
(asf_st->ds_packet_size / asf_st->ds_chunk_size <= 1) ||
asf_st->ds_packet_size % asf_st->ds_chunk_size)
asf_st->ds_span = 0; 
}
} else if (type == AVMEDIA_TYPE_VIDEO &&
size - (avio_tell(pb) - pos1 + 24) >= 51) {
avio_rl32(pb);
avio_rl32(pb);
avio_r8(pb);
avio_rl16(pb); 
sizeX = avio_rl32(pb); 
st->codecpar->width = avio_rl32(pb);
st->codecpar->height = avio_rl32(pb);
avio_rl16(pb); 
st->codecpar->bits_per_coded_sample = avio_rl16(pb); 
tag1 = avio_rl32(pb);
avio_skip(pb, 20);
if (sizeX > 40) {
st->codecpar->extradata_size = ffio_limit(pb, sizeX - 40);
st->codecpar->extradata = av_mallocz(st->codecpar->extradata_size +
AV_INPUT_BUFFER_PADDING_SIZE);
if (!st->codecpar->extradata)
return AVERROR(ENOMEM);
avio_read(pb, st->codecpar->extradata, st->codecpar->extradata_size);
}
if (st->codecpar->extradata_size && (st->codecpar->bits_per_coded_sample <= 8)) {
#if HAVE_BIGENDIAN
int i;
for (i = 0; i < FFMIN(st->codecpar->extradata_size, AVPALETTE_SIZE) / 4; i++)
asf_st->palette[i] = av_bswap32(((uint32_t *)st->codecpar->extradata)[i]);
#else
memcpy(asf_st->palette, st->codecpar->extradata,
FFMIN(st->codecpar->extradata_size, AVPALETTE_SIZE));
#endif
asf_st->palette_changed = 1;
}
st->codecpar->codec_tag = tag1;
st->codecpar->codec_id = ff_codec_get_id(ff_codec_bmp_tags, tag1);
if (tag1 == MKTAG('D', 'V', 'R', ' ')) {
st->need_parsing = AVSTREAM_PARSE_FULL;
st->codecpar->width =
st->codecpar->height = 0;
av_freep(&st->codecpar->extradata);
st->codecpar->extradata_size = 0;
}
if (st->codecpar->codec_id == AV_CODEC_ID_H264)
st->need_parsing = AVSTREAM_PARSE_FULL_ONCE;
if (st->codecpar->codec_id == AV_CODEC_ID_MPEG4)
st->need_parsing = AVSTREAM_PARSE_FULL_ONCE;
}
pos2 = avio_tell(pb);
avio_skip(pb, size - (pos2 - pos1 + 24));
return 0;
}
static int asf_read_ext_stream_properties(AVFormatContext *s, int64_t size)
{
ASFContext *asf = s->priv_data;
AVIOContext *pb = s->pb;
ff_asf_guid g;
int ext_len, payload_ext_ct, stream_ct, i;
uint32_t leak_rate, stream_num;
unsigned int stream_languageid_index;
avio_rl64(pb); 
avio_rl64(pb); 
leak_rate = avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
stream_num = avio_rl16(pb); 
stream_languageid_index = avio_rl16(pb); 
if (stream_num < 128)
asf->streams[stream_num].stream_language_index = stream_languageid_index;
avio_rl64(pb); 
stream_ct = avio_rl16(pb); 
payload_ext_ct = avio_rl16(pb); 
if (stream_num < 128) {
asf->stream_bitrates[stream_num] = leak_rate;
asf->streams[stream_num].payload_ext_ct = 0;
}
for (i = 0; i < stream_ct; i++) {
avio_rl16(pb);
ext_len = avio_rl16(pb);
avio_skip(pb, ext_len);
}
for (i = 0; i < payload_ext_ct; i++) {
int size;
ff_get_guid(pb, &g);
size = avio_rl16(pb);
ext_len = avio_rl32(pb);
avio_skip(pb, ext_len);
if (stream_num < 128 && i < FF_ARRAY_ELEMS(asf->streams[stream_num].payload)) {
ASFPayload *p = &asf->streams[stream_num].payload[i];
p->type = g[0];
p->size = size;
av_log(s, AV_LOG_DEBUG, "Payload extension %x %d\n", g[0], p->size );
asf->streams[stream_num].payload_ext_ct ++;
}
}
return 0;
}
static int asf_read_content_desc(AVFormatContext *s, int64_t size)
{
AVIOContext *pb = s->pb;
int len1, len2, len3, len4, len5;
len1 = avio_rl16(pb);
len2 = avio_rl16(pb);
len3 = avio_rl16(pb);
len4 = avio_rl16(pb);
len5 = avio_rl16(pb);
get_tag(s, "title", 0, len1, 32);
get_tag(s, "author", 0, len2, 32);
get_tag(s, "copyright", 0, len3, 32);
get_tag(s, "comment", 0, len4, 32);
avio_skip(pb, len5);
return 0;
}
static int asf_read_ext_content_desc(AVFormatContext *s, int64_t size)
{
AVIOContext *pb = s->pb;
ASFContext *asf = s->priv_data;
int desc_count, i, ret;
desc_count = avio_rl16(pb);
for (i = 0; i < desc_count; i++) {
int name_len, value_type, value_len;
char name[1024];
name_len = avio_rl16(pb);
if (name_len % 2) 
name_len += 1;
if ((ret = avio_get_str16le(pb, name_len, name, sizeof(name))) < name_len)
avio_skip(pb, name_len - ret);
value_type = avio_rl16(pb);
value_len = avio_rl16(pb);
if (!value_type && value_len % 2)
value_len += 1;
if (!strcmp(name, "AspectRatioX"))
asf->dar[0].num = get_value(s->pb, value_type, 32);
else if (!strcmp(name, "AspectRatioY"))
asf->dar[0].den = get_value(s->pb, value_type, 32);
else
get_tag(s, name, value_type, value_len, 32);
}
return 0;
}
static int asf_read_language_list(AVFormatContext *s, int64_t size)
{
AVIOContext *pb = s->pb;
ASFContext *asf = s->priv_data;
int j, ret;
int stream_count = avio_rl16(pb);
for (j = 0; j < stream_count; j++) {
char lang[6];
unsigned int lang_len = avio_r8(pb);
if ((ret = avio_get_str16le(pb, lang_len, lang,
sizeof(lang))) < lang_len)
avio_skip(pb, lang_len - ret);
if (j < 128)
av_strlcpy(asf->stream_languages[j], lang,
sizeof(*asf->stream_languages));
}
return 0;
}
static int asf_read_metadata(AVFormatContext *s, int64_t size)
{
AVIOContext *pb = s->pb;
ASFContext *asf = s->priv_data;
int n, stream_num, name_len_utf16, name_len_utf8, value_len;
int ret, i;
n = avio_rl16(pb);
for (i = 0; i < n; i++) {
uint8_t *name;
int value_type;
avio_rl16(pb); 
stream_num = avio_rl16(pb);
name_len_utf16 = avio_rl16(pb);
value_type = avio_rl16(pb); 
value_len = avio_rl32(pb);
if (value_len < 0 || value_len > UINT16_MAX)
return AVERROR_INVALIDDATA;
name_len_utf8 = 2*name_len_utf16 + 1;
name = av_malloc(name_len_utf8);
if (!name)
return AVERROR(ENOMEM);
if ((ret = avio_get_str16le(pb, name_len_utf16, name, name_len_utf8)) < name_len_utf16)
avio_skip(pb, name_len_utf16 - ret);
av_log(s, AV_LOG_TRACE, "%d stream %d name_len %2d type %d len %4d <%s>\n",
i, stream_num, name_len_utf16, value_type, value_len, name);
if (!strcmp(name, "AspectRatioX")){
int aspect_x = get_value(s->pb, value_type, 16);
if(stream_num < 128)
asf->dar[stream_num].num = aspect_x;
} else if(!strcmp(name, "AspectRatioY")){
int aspect_y = get_value(s->pb, value_type, 16);
if(stream_num < 128)
asf->dar[stream_num].den = aspect_y;
} else {
get_tag(s, name, value_type, value_len, 16);
}
av_freep(&name);
}
return 0;
}
static int asf_read_marker(AVFormatContext *s, int64_t size)
{
AVIOContext *pb = s->pb;
ASFContext *asf = s->priv_data;
int i, count, name_len, ret;
char name[1024];
avio_rl64(pb); 
avio_rl64(pb); 
count = avio_rl32(pb); 
avio_rl16(pb); 
name_len = avio_rl16(pb); 
avio_skip(pb, name_len);
for (i = 0; i < count; i++) {
int64_t pres_time;
int name_len;
if (avio_feof(pb))
return AVERROR_INVALIDDATA;
avio_rl64(pb); 
pres_time = avio_rl64(pb); 
pres_time -= asf->hdr.preroll * 10000;
avio_rl16(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
name_len = avio_rl32(pb); 
if ((ret = avio_get_str16le(pb, name_len * 2, name,
sizeof(name))) < name_len)
avio_skip(pb, name_len - ret);
avpriv_new_chapter(s, i, (AVRational) { 1, 10000000 }, pres_time,
AV_NOPTS_VALUE, name);
}
return 0;
}
static int asf_read_header(AVFormatContext *s)
{
ASFContext *asf = s->priv_data;
ff_asf_guid g;
AVIOContext *pb = s->pb;
int i;
int64_t gsize;
ff_get_guid(pb, &g);
if (ff_guidcmp(&g, &ff_asf_header))
return AVERROR_INVALIDDATA;
avio_rl64(pb);
avio_rl32(pb);
avio_r8(pb);
avio_r8(pb);
memset(&asf->asfid2avid, -1, sizeof(asf->asfid2avid));
for (i = 0; i<128; i++)
asf->streams[i].stream_language_index = 128; 
for (;;) {
uint64_t gpos = avio_tell(pb);
int ret = 0;
ff_get_guid(pb, &g);
gsize = avio_rl64(pb);
print_guid(&g);
if (!ff_guidcmp(&g, &ff_asf_data_header)) {
asf->data_object_offset = avio_tell(pb);
if (!(asf->hdr.flags & 0x01) && gsize >= 100)
asf->data_object_size = gsize - 24;
else
asf->data_object_size = (uint64_t)-1;
break;
}
if (gsize < 24)
return AVERROR_INVALIDDATA;
if (!ff_guidcmp(&g, &ff_asf_file_header)) {
ret = asf_read_file_properties(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_stream_header)) {
ret = asf_read_stream_properties(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_comment_header)) {
asf_read_content_desc(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_language_guid)) {
asf_read_language_list(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_extended_content_header)) {
asf_read_ext_content_desc(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_metadata_header)) {
asf_read_metadata(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_metadata_library_header)) {
asf_read_metadata(s, gsize);
} else if (!ff_guidcmp(&g, &ff_asf_ext_stream_header)) {
asf_read_ext_stream_properties(s, gsize);
continue;
} else if (!ff_guidcmp(&g, &ff_asf_head1_guid)) {
ff_get_guid(pb, &g);
avio_skip(pb, 6);
continue;
} else if (!ff_guidcmp(&g, &ff_asf_marker_header)) {
asf_read_marker(s, gsize);
} else if (avio_feof(pb)) {
return AVERROR_EOF;
} else {
if (!s->keylen) {
if (!ff_guidcmp(&g, &ff_asf_content_encryption)) {
unsigned int len;
AVPacket pkt;
av_log(s, AV_LOG_WARNING,
"DRM protected stream detected, decoding will likely fail!\n");
len= avio_rl32(pb);
av_log(s, AV_LOG_DEBUG, "Secret data:\n");
if ((ret = av_get_packet(pb, &pkt, len)) < 0)
return ret;
av_hex_dump_log(s, AV_LOG_DEBUG, pkt.data, pkt.size);
av_packet_unref(&pkt);
len= avio_rl32(pb);
if (len > UINT16_MAX)
return AVERROR_INVALIDDATA;
get_tag(s, "ASF_Protection_Type", -1, len, 32);
len= avio_rl32(pb);
if (len > UINT16_MAX)
return AVERROR_INVALIDDATA;
get_tag(s, "ASF_Key_ID", -1, len, 32);
len= avio_rl32(pb);
if (len > UINT16_MAX)
return AVERROR_INVALIDDATA;
get_tag(s, "ASF_License_URL", -1, len, 32);
} else if (!ff_guidcmp(&g, &ff_asf_ext_content_encryption)) {
av_log(s, AV_LOG_WARNING,
"Ext DRM protected stream detected, decoding will likely fail!\n");
av_dict_set(&s->metadata, "encryption", "ASF Extended Content Encryption", 0);
} else if (!ff_guidcmp(&g, &ff_asf_digital_signature)) {
av_log(s, AV_LOG_INFO, "Digital signature detected!\n");
}
}
}
if (ret < 0)
return ret;
if (avio_tell(pb) != gpos + gsize)
av_log(s, AV_LOG_DEBUG,
"gpos mismatch our pos=%"PRIu64", end=%"PRId64"\n",
avio_tell(pb) - gpos, gsize);
avio_seek(pb, gpos + gsize, SEEK_SET);
}
ff_get_guid(pb, &g);
avio_rl64(pb);
avio_r8(pb);
avio_r8(pb);
if (avio_feof(pb))
return AVERROR_EOF;
asf->data_offset = avio_tell(pb);
asf->packet_size_left = 0;
for (i = 0; i < 128; i++) {
int stream_num = asf->asfid2avid[i];
if (stream_num >= 0) {
AVStream *st = s->streams[stream_num];
if (!st->codecpar->bit_rate)
st->codecpar->bit_rate = asf->stream_bitrates[i];
if (asf->dar[i].num > 0 && asf->dar[i].den > 0) {
av_reduce(&st->sample_aspect_ratio.num,
&st->sample_aspect_ratio.den,
asf->dar[i].num, asf->dar[i].den, INT_MAX);
} else if ((asf->dar[0].num > 0) && (asf->dar[0].den > 0) &&
(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO))
av_reduce(&st->sample_aspect_ratio.num,
&st->sample_aspect_ratio.den,
asf->dar[0].num, asf->dar[0].den, INT_MAX);
av_log(s, AV_LOG_TRACE, "i=%d, st->codecpar->codec_type:%d, asf->dar %d:%d sar=%d:%d\n",
i, st->codecpar->codec_type, asf->dar[i].num, asf->dar[i].den,
st->sample_aspect_ratio.num, st->sample_aspect_ratio.den);
if (asf->streams[i].stream_language_index < 128) {
const char *rfc1766 = asf->stream_languages[asf->streams[i].stream_language_index];
if (rfc1766 && strlen(rfc1766) > 1) {
const char primary_tag[3] = { rfc1766[0], rfc1766[1], '\0' }; 
const char *iso6392 = ff_convert_lang_to(primary_tag,
AV_LANG_ISO639_2_BIBL);
if (iso6392)
av_dict_set(&st->metadata, "language", iso6392, 0);
}
}
}
}
ff_metadata_conv(&s->metadata, NULL, ff_asf_metadata_conv);
return 0;
}
#define DO_2BITS(bits, var, defval) switch (bits & 3) { case 3: var = avio_rl32(pb); rsize += 4; break; case 2: var = avio_rl16(pb); rsize += 2; break; case 1: var = avio_r8(pb); rsize++; break; default: var = defval; break; }
static int asf_get_packet(AVFormatContext *s, AVIOContext *pb)
{
ASFContext *asf = s->priv_data;
uint32_t packet_length, padsize;
int rsize = 8;
int c, d, e, off;
if (asf->uses_std_ecc > 0) {
off = 32768;
if (asf->no_resync_search)
off = 3;
c = d = e = -1;
while (off-- > 0) {
c = d;
d = e;
e = avio_r8(pb);
if (c == 0x82 && !d && !e)
break;
}
if (c != 0x82) {
if (pb->error == AVERROR(EAGAIN))
return AVERROR(EAGAIN);
if (!avio_feof(pb))
av_log(s, AV_LOG_ERROR,
"ff asf bad header %x at:%"PRId64"\n", c, avio_tell(pb));
}
if ((c & 0x8f) == 0x82) {
if (d || e) {
if (!avio_feof(pb))
av_log(s, AV_LOG_ERROR, "ff asf bad non zero\n");
return AVERROR_INVALIDDATA;
}
c = avio_r8(pb);
d = avio_r8(pb);
rsize += 3;
} else if(!avio_feof(pb)) {
avio_seek(pb, -1, SEEK_CUR); 
}
} else {
c = avio_r8(pb);
if (c & 0x80) {
rsize ++;
if (!(c & 0x60)) {
d = avio_r8(pb);
e = avio_r8(pb);
avio_seek(pb, (c & 0xF) - 2, SEEK_CUR);
rsize += c & 0xF;
}
if (c != 0x82)
avpriv_request_sample(s, "Invalid ECC byte");
if (!asf->uses_std_ecc)
asf->uses_std_ecc = (c == 0x82 && !d && !e) ? 1 : -1;
c = avio_r8(pb);
} else
asf->uses_std_ecc = -1;
d = avio_r8(pb);
}
asf->packet_flags = c;
asf->packet_property = d;
DO_2BITS(asf->packet_flags >> 5, packet_length, s->packet_size);
DO_2BITS(asf->packet_flags >> 1, padsize, 0); 
DO_2BITS(asf->packet_flags >> 3, padsize, 0); 
if (!packet_length || packet_length >= (1U << 29)) {
av_log(s, AV_LOG_ERROR,
"invalid packet_length %"PRIu32" at:%"PRId64"\n",
packet_length, avio_tell(pb));
return AVERROR_INVALIDDATA;
}
if (padsize >= packet_length) {
av_log(s, AV_LOG_ERROR,
"invalid padsize %"PRIu32" at:%"PRId64"\n", padsize, avio_tell(pb));
return AVERROR_INVALIDDATA;
}
asf->packet_timestamp = avio_rl32(pb);
avio_rl16(pb); 
if (asf->packet_flags & 0x01) {
asf->packet_segsizetype = avio_r8(pb);
rsize++;
asf->packet_segments = asf->packet_segsizetype & 0x3f;
} else {
asf->packet_segments = 1;
asf->packet_segsizetype = 0x80;
}
if (rsize > packet_length - padsize) {
asf->packet_size_left = 0;
av_log(s, AV_LOG_ERROR,
"invalid packet header length %d for pktlen %"PRIu32"-%"PRIu32" at %"PRId64"\n",
rsize, packet_length, padsize, avio_tell(pb));
return AVERROR_INVALIDDATA;
}
asf->packet_size_left = packet_length - padsize - rsize;
if (packet_length < asf->hdr.min_pktsize)
padsize += asf->hdr.min_pktsize - packet_length;
asf->packet_padsize = padsize;
av_log(s, AV_LOG_TRACE, "packet: size=%d padsize=%d left=%d\n",
s->packet_size, asf->packet_padsize, asf->packet_size_left);
return 0;
}
static int asf_read_frame_header(AVFormatContext *s, AVIOContext *pb)
{
ASFContext *asf = s->priv_data;
ASFStream *asfst;
int rsize = 1;
int num = avio_r8(pb);
int i;
int64_t ts0, ts1 av_unused;
asf->packet_segments--;
asf->packet_key_frame = num >> 7;
asf->stream_index = asf->asfid2avid[num & 0x7f];
asfst = &asf->streams[num & 0x7f];
DO_2BITS(asf->packet_property >> 4, asf->packet_seq, 0);
DO_2BITS(asf->packet_property >> 2, asf->packet_frag_offset, 0);
DO_2BITS(asf->packet_property, asf->packet_replic_size, 0);
av_log(asf, AV_LOG_TRACE, "key:%d stream:%d seq:%d offset:%d replic_size:%d num:%X packet_property %X\n",
asf->packet_key_frame, asf->stream_index, asf->packet_seq,
asf->packet_frag_offset, asf->packet_replic_size, num, asf->packet_property);
if (rsize+(int64_t)asf->packet_replic_size > asf->packet_size_left) {
av_log(s, AV_LOG_ERROR, "packet_replic_size %d is invalid\n", asf->packet_replic_size);
return AVERROR_INVALIDDATA;
}
if (asf->packet_replic_size >= 8) {
int64_t end = avio_tell(pb) + asf->packet_replic_size;
AVRational aspect;
asfst->packet_obj_size = avio_rl32(pb);
if (asfst->packet_obj_size >= (1 << 24) || asfst->packet_obj_size < 0) {
av_log(s, AV_LOG_ERROR, "packet_obj_size %d invalid\n", asfst->packet_obj_size);
asfst->packet_obj_size = 0;
return AVERROR_INVALIDDATA;
}
asf->packet_frag_timestamp = avio_rl32(pb); 
for (i = 0; i < asfst->payload_ext_ct; i++) {
ASFPayload *p = &asfst->payload[i];
int size = p->size;
int64_t payend;
if (size == 0xFFFF)
size = avio_rl16(pb);
payend = avio_tell(pb) + size;
if (payend > end) {
av_log(s, AV_LOG_ERROR, "too long payload\n");
break;
}
switch (p->type) {
case 0x50:
break;
case 0x54:
aspect.num = avio_r8(pb);
aspect.den = avio_r8(pb);
if (aspect.num > 0 && aspect.den > 0 && asf->stream_index >= 0) {
s->streams[asf->stream_index]->sample_aspect_ratio = aspect;
}
break;
case 0x2A:
avio_skip(pb, 8);
ts0 = avio_rl64(pb);
ts1 = avio_rl64(pb);
if (ts0!= -1) asf->packet_frag_timestamp = ts0/10000;
else asf->packet_frag_timestamp = AV_NOPTS_VALUE;
asf->ts_is_pts = 1;
break;
case 0x5B:
case 0xB7:
case 0xCC:
case 0xC0:
case 0xA0:
break;
}
avio_seek(pb, payend, SEEK_SET);
}
avio_seek(pb, end, SEEK_SET);
rsize += asf->packet_replic_size; 
} else if (asf->packet_replic_size == 1) {
asf->packet_time_start = asf->packet_frag_offset;
asf->packet_frag_offset = 0;
asf->packet_frag_timestamp = asf->packet_timestamp;
asf->packet_time_delta = avio_r8(pb);
rsize++;
} else if (asf->packet_replic_size != 0) {
av_log(s, AV_LOG_ERROR, "unexpected packet_replic_size of %d\n",
asf->packet_replic_size);
return AVERROR_INVALIDDATA;
}
if (asf->packet_flags & 0x01) {
DO_2BITS(asf->packet_segsizetype >> 6, asf->packet_frag_size, 0); 
if (rsize > asf->packet_size_left) {
av_log(s, AV_LOG_ERROR, "packet_replic_size is invalid\n");
return AVERROR_INVALIDDATA;
} else if (asf->packet_frag_size > asf->packet_size_left - rsize) {
if (asf->packet_frag_size > asf->packet_size_left - rsize + asf->packet_padsize) {
av_log(s, AV_LOG_ERROR, "packet_frag_size is invalid (%d>%d-%d+%d)\n",
asf->packet_frag_size, asf->packet_size_left, rsize, asf->packet_padsize);
return AVERROR_INVALIDDATA;
} else {
int diff = asf->packet_frag_size - (asf->packet_size_left - rsize);
asf->packet_size_left += diff;
asf->packet_padsize -= diff;
}
}
} else {
asf->packet_frag_size = asf->packet_size_left - rsize;
}
if (asf->packet_replic_size == 1) {
asf->packet_multi_size = asf->packet_frag_size;
if (asf->packet_multi_size > asf->packet_size_left)
return AVERROR_INVALIDDATA;
}
asf->packet_size_left -= rsize;
return 0;
}
static int asf_parse_packet(AVFormatContext *s, AVIOContext *pb, AVPacket *pkt)
{
ASFContext *asf = s->priv_data;
ASFStream *asf_st = 0;
for (;;) {
int ret;
if (avio_feof(pb))
return AVERROR_EOF;
if (asf->packet_size_left < FRAME_HEADER_SIZE ||
asf->packet_segments < 1 && asf->packet_time_start == 0) {
int ret = asf->packet_size_left + asf->packet_padsize;
if (asf->packet_size_left && asf->packet_size_left < FRAME_HEADER_SIZE)
av_log(s, AV_LOG_WARNING, "Skip due to FRAME_HEADER_SIZE\n");
assert(ret >= 0);
avio_skip(pb, ret);
asf->packet_pos = avio_tell(pb);
if (asf->data_object_size != (uint64_t)-1 &&
(asf->packet_pos - asf->data_object_offset >= asf->data_object_size))
return AVERROR_EOF; 
return 1;
}
if (asf->packet_time_start == 0) {
if (asf_read_frame_header(s, pb) < 0) {
asf->packet_time_start = asf->packet_segments = 0;
continue;
}
if (asf->stream_index < 0 ||
s->streams[asf->stream_index]->discard >= AVDISCARD_ALL ||
(!asf->packet_key_frame &&
(s->streams[asf->stream_index]->discard >= AVDISCARD_NONKEY || asf->streams[s->streams[asf->stream_index]->id].skip_to_key))) {
asf->packet_time_start = 0;
avio_skip(pb, asf->packet_frag_size);
asf->packet_size_left -= asf->packet_frag_size;
if (asf->stream_index < 0)
av_log(s, AV_LOG_ERROR, "ff asf skip %d (unknown stream)\n",
asf->packet_frag_size);
continue;
}
asf->asf_st = &asf->streams[s->streams[asf->stream_index]->id];
if (!asf->packet_frag_offset)
asf->asf_st->skip_to_key = 0;
}
asf_st = asf->asf_st;
av_assert0(asf_st);
if (!asf_st->frag_offset && asf->packet_frag_offset) {
av_log(s, AV_LOG_TRACE, "skipping asf data pkt with fragment offset for "
"stream:%d, expected:%d but got %d from pkt)\n",
asf->stream_index, asf_st->frag_offset,
asf->packet_frag_offset);
avio_skip(pb, asf->packet_frag_size);
asf->packet_size_left -= asf->packet_frag_size;
continue;
}
if (asf->packet_replic_size == 1) {
asf->packet_frag_timestamp = asf->packet_time_start;
asf->packet_time_start += asf->packet_time_delta;
asf_st->packet_obj_size = asf->packet_frag_size = avio_r8(pb);
asf->packet_size_left--;
asf->packet_multi_size--;
if (asf->packet_multi_size < asf_st->packet_obj_size) {
asf->packet_time_start = 0;
avio_skip(pb, asf->packet_multi_size);
asf->packet_size_left -= asf->packet_multi_size;
continue;
}
asf->packet_multi_size -= asf_st->packet_obj_size;
}
if (asf_st->pkt.size != asf_st->packet_obj_size ||
asf_st->frag_offset + asf->packet_frag_size > asf_st->pkt.size) {
int ret;
if (asf_st->pkt.data) {
av_log(s, AV_LOG_INFO,
"freeing incomplete packet size %d, new %d\n",
asf_st->pkt.size, asf_st->packet_obj_size);
asf_st->frag_offset = 0;
av_packet_unref(&asf_st->pkt);
}
if ((ret = av_new_packet(&asf_st->pkt, asf_st->packet_obj_size)) < 0)
return ret;
asf_st->seq = asf->packet_seq;
if (asf->ts_is_pts) {
asf_st->pkt.pts = asf->packet_frag_timestamp - asf->hdr.preroll;
} else
asf_st->pkt.dts = asf->packet_frag_timestamp - asf->hdr.preroll;
asf_st->pkt.stream_index = asf->stream_index;
asf_st->pkt.pos = asf_st->packet_pos = asf->packet_pos;
asf_st->pkt_clean = 0;
if (asf_st->pkt.data && asf_st->palette_changed) {
uint8_t *pal;
pal = av_packet_new_side_data(&asf_st->pkt, AV_PKT_DATA_PALETTE,
AVPALETTE_SIZE);
if (!pal) {
av_log(s, AV_LOG_ERROR, "Cannot append palette to packet\n");
} else {
memcpy(pal, asf_st->palette, AVPALETTE_SIZE);
asf_st->palette_changed = 0;
}
}
av_log(asf, AV_LOG_TRACE, "new packet: stream:%d key:%d packet_key:%d audio:%d size:%d\n",
asf->stream_index, asf->packet_key_frame,
asf_st->pkt.flags & AV_PKT_FLAG_KEY,
s->streams[asf->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO,
asf_st->packet_obj_size);
if (s->streams[asf->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
asf->packet_key_frame = 1;
if (asf->packet_key_frame)
asf_st->pkt.flags |= AV_PKT_FLAG_KEY;
}
av_log(asf, AV_LOG_TRACE, "READ PACKET s:%d os:%d o:%d,%d l:%d DATA:%p\n",
s->packet_size, asf_st->pkt.size, asf->packet_frag_offset,
asf_st->frag_offset, asf->packet_frag_size, asf_st->pkt.data);
asf->packet_size_left -= asf->packet_frag_size;
if (asf->packet_size_left < 0)
continue;
if (asf->packet_frag_offset >= asf_st->pkt.size ||
asf->packet_frag_size > asf_st->pkt.size - asf->packet_frag_offset) {
av_log(s, AV_LOG_ERROR,
"packet fragment position invalid %u,%u not in %u\n",
asf->packet_frag_offset, asf->packet_frag_size,
asf_st->pkt.size);
continue;
}
if (asf->packet_frag_offset != asf_st->frag_offset && !asf_st->pkt_clean) {
memset(asf_st->pkt.data + asf_st->frag_offset, 0, asf_st->pkt.size - asf_st->frag_offset);
asf_st->pkt_clean = 1;
}
ret = avio_read(pb, asf_st->pkt.data + asf->packet_frag_offset,
asf->packet_frag_size);
if (ret != asf->packet_frag_size) {
if (ret < 0 || asf->packet_frag_offset + ret == 0)
return ret < 0 ? ret : AVERROR_EOF;
if (asf_st->ds_span > 1) {
memset(asf_st->pkt.data + asf->packet_frag_offset + ret, 0,
asf->packet_frag_size - ret);
ret = asf->packet_frag_size;
} else {
av_shrink_packet(&asf_st->pkt, asf->packet_frag_offset + ret);
}
}
if (s->key && s->keylen == 20)
ff_asfcrypt_dec(s->key, asf_st->pkt.data + asf->packet_frag_offset,
ret);
asf_st->frag_offset += ret;
if (asf_st->frag_offset == asf_st->pkt.size) {
if (s->streams[asf->stream_index]->codecpar->codec_id == AV_CODEC_ID_MPEG2VIDEO &&
asf_st->pkt.size > 100) {
int i;
for (i = 0; i < asf_st->pkt.size && !asf_st->pkt.data[i]; i++)
;
if (i == asf_st->pkt.size) {
av_log(s, AV_LOG_DEBUG, "discarding ms fart\n");
asf_st->frag_offset = 0;
av_packet_unref(&asf_st->pkt);
continue;
}
}
if (asf_st->ds_span > 1) {
if (asf_st->pkt.size != asf_st->ds_packet_size * asf_st->ds_span) {
av_log(s, AV_LOG_ERROR,
"pkt.size != ds_packet_size * ds_span (%d %d %d)\n",
asf_st->pkt.size, asf_st->ds_packet_size,
asf_st->ds_span);
} else {
AVBufferRef *buf = av_buffer_alloc(asf_st->pkt.size +
AV_INPUT_BUFFER_PADDING_SIZE);
if (buf) {
uint8_t *newdata = buf->data;
int offset = 0;
memset(newdata + asf_st->pkt.size, 0,
AV_INPUT_BUFFER_PADDING_SIZE);
while (offset < asf_st->pkt.size) {
int off = offset / asf_st->ds_chunk_size;
int row = off / asf_st->ds_span;
int col = off % asf_st->ds_span;
int idx = row + col * asf_st->ds_packet_size / asf_st->ds_chunk_size;
assert(offset + asf_st->ds_chunk_size <= asf_st->pkt.size);
assert(idx + 1 <= asf_st->pkt.size / asf_st->ds_chunk_size);
memcpy(newdata + offset,
asf_st->pkt.data + idx * asf_st->ds_chunk_size,
asf_st->ds_chunk_size);
offset += asf_st->ds_chunk_size;
}
av_buffer_unref(&asf_st->pkt.buf);
asf_st->pkt.buf = buf;
asf_st->pkt.data = buf->data;
}
}
}
asf_st->frag_offset = 0;
*pkt = asf_st->pkt;
asf_st->pkt.buf = 0;
asf_st->pkt.size = 0;
asf_st->pkt.data = 0;
asf_st->pkt.side_data_elems = 0;
asf_st->pkt.side_data = NULL;
break; 
}
}
return 0;
}
static int asf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
ASFContext *asf = s->priv_data;
for (;;) {
int ret;
if ((ret = asf_parse_packet(s, s->pb, pkt)) <= 0)
return ret;
if ((ret = asf_get_packet(s, s->pb)) < 0)
assert(asf->packet_size_left < FRAME_HEADER_SIZE ||
asf->packet_segments < 1);
asf->packet_time_start = 0;
}
}
static void asf_reset_header(AVFormatContext *s)
{
ASFContext *asf = s->priv_data;
ASFStream *asf_st;
int i;
asf->packet_size_left = 0;
asf->packet_flags = 0;
asf->packet_property = 0;
asf->packet_timestamp = 0;
asf->packet_segsizetype = 0;
asf->packet_segments = 0;
asf->packet_seq = 0;
asf->packet_replic_size = 0;
asf->packet_key_frame = 0;
asf->packet_padsize = 0;
asf->packet_frag_offset = 0;
asf->packet_frag_size = 0;
asf->packet_frag_timestamp = 0;
asf->packet_multi_size = 0;
asf->packet_time_delta = 0;
asf->packet_time_start = 0;
for (i = 0; i < 128; i++) {
asf_st = &asf->streams[i];
av_packet_unref(&asf_st->pkt);
asf_st->packet_obj_size = 0;
asf_st->frag_offset = 0;
asf_st->seq = 0;
}
asf->asf_st = NULL;
}
static void skip_to_key(AVFormatContext *s)
{
ASFContext *asf = s->priv_data;
int i;
for (i = 0; i < 128; i++) {
int j = asf->asfid2avid[i];
ASFStream *asf_st = &asf->streams[i];
if (j < 0 || s->streams[j]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
continue;
asf_st->skip_to_key = 1;
}
}
static int asf_read_close(AVFormatContext *s)
{
asf_reset_header(s);
return 0;
}
static int64_t asf_read_pts(AVFormatContext *s, int stream_index,
int64_t *ppos, int64_t pos_limit)
{
ASFContext *asf = s->priv_data;
AVPacket pkt1, *pkt = &pkt1;
ASFStream *asf_st;
int64_t pts;
int64_t pos = *ppos;
int i;
int64_t start_pos[ASF_MAX_STREAMS];
for (i = 0; i < s->nb_streams; i++)
start_pos[i] = pos;
if (s->packet_size > 0)
pos = (pos + s->packet_size - 1 - s->internal->data_offset) /
s->packet_size * s->packet_size +
s->internal->data_offset;
*ppos = pos;
if (avio_seek(s->pb, pos, SEEK_SET) < 0)
return AV_NOPTS_VALUE;
ff_read_frame_flush(s);
asf_reset_header(s);
for (;;) {
if (av_read_frame(s, pkt) < 0) {
av_log(s, AV_LOG_INFO, "asf_read_pts failed\n");
return AV_NOPTS_VALUE;
}
pts = pkt->dts;
if (pkt->flags & AV_PKT_FLAG_KEY) {
i = pkt->stream_index;
asf_st = &asf->streams[s->streams[i]->id];
pos = asf_st->packet_pos;
av_assert1(pkt->pos == asf_st->packet_pos);
av_add_index_entry(s->streams[i], pos, pts, pkt->size,
pos - start_pos[i] + 1, AVINDEX_KEYFRAME);
start_pos[i] = asf_st->packet_pos + 1;
if (pkt->stream_index == stream_index) {
av_packet_unref(pkt);
break;
}
}
av_packet_unref(pkt);
}
*ppos = pos;
return pts;
}
static int asf_build_simple_index(AVFormatContext *s, int stream_index)
{
ff_asf_guid g;
ASFContext *asf = s->priv_data;
int64_t current_pos = avio_tell(s->pb);
int64_t ret;
if((ret = avio_seek(s->pb, asf->data_object_offset + asf->data_object_size, SEEK_SET)) < 0) {
return ret;
}
if ((ret = ff_get_guid(s->pb, &g)) < 0)
goto end;
while (ff_guidcmp(&g, &ff_asf_simple_index_header)) {
int64_t gsize = avio_rl64(s->pb);
if (gsize < 24 || avio_feof(s->pb)) {
goto end;
}
avio_skip(s->pb, gsize - 24);
if ((ret = ff_get_guid(s->pb, &g)) < 0)
goto end;
}
{
int64_t itime, last_pos = -1;
int pct, ict;
int i;
int64_t av_unused gsize = avio_rl64(s->pb);
if ((ret = ff_get_guid(s->pb, &g)) < 0)
goto end;
itime = avio_rl64(s->pb);
pct = avio_rl32(s->pb);
ict = avio_rl32(s->pb);
av_log(s, AV_LOG_DEBUG,
"itime:0x%"PRIx64", pct:%d, ict:%d\n", itime, pct, ict);
for (i = 0; i < ict; i++) {
int pktnum = avio_rl32(s->pb);
int pktct = avio_rl16(s->pb);
int64_t pos = s->internal->data_offset + s->packet_size * (int64_t)pktnum;
int64_t index_pts = FFMAX(av_rescale(itime, i, 10000) - asf->hdr.preroll, 0);
if (avio_feof(s->pb)) {
ret = AVERROR_INVALIDDATA;
goto end;
}
if (pos != last_pos) {
av_log(s, AV_LOG_DEBUG, "pktnum:%d, pktct:%d pts: %"PRId64"\n",
pktnum, pktct, index_pts);
av_add_index_entry(s->streams[stream_index], pos, index_pts,
s->packet_size, 0, AVINDEX_KEYFRAME);
last_pos = pos;
}
}
asf->index_read = ict > 1;
}
end:
avio_seek(s->pb, current_pos, SEEK_SET);
return ret;
}
static int asf_read_seek(AVFormatContext *s, int stream_index,
int64_t pts, int flags)
{
ASFContext *asf = s->priv_data;
AVStream *st = s->streams[stream_index];
int ret = 0;
if (s->packet_size <= 0)
return -1;
if (s->pb) {
int64_t ret = avio_seek_time(s->pb, stream_index, pts, flags);
if (ret >= 0)
asf_reset_header(s);
if (ret != AVERROR(ENOSYS))
return ret;
}
if (!pts) {
asf_reset_header(s);
avio_seek(s->pb, s->internal->data_offset, SEEK_SET);
return 0;
}
if (!asf->index_read) {
ret = asf_build_simple_index(s, stream_index);
if (ret < 0)
asf->index_read = -1;
}
if (asf->index_read > 0 && st->index_entries) {
int index = av_index_search_timestamp(st, pts, flags);
if (index >= 0) {
uint64_t pos = st->index_entries[index].pos;
av_log(s, AV_LOG_DEBUG, "SEEKTO: %"PRId64"\n", pos);
if(avio_seek(s->pb, pos, SEEK_SET) < 0)
return -1;
asf_reset_header(s);
skip_to_key(s);
return 0;
}
}
if (ff_seek_frame_binary(s, stream_index, pts, flags) < 0)
return -1;
asf_reset_header(s);
skip_to_key(s);
return 0;
}
AVInputFormat ff_asf_demuxer = {
.name = "asf",
.long_name = NULL_IF_CONFIG_SMALL("ASF (Advanced / Active Streaming Format)"),
.priv_data_size = sizeof(ASFContext),
.read_probe = asf_probe,
.read_header = asf_read_header,
.read_packet = asf_read_packet,
.read_close = asf_read_close,
.read_seek = asf_read_seek,
.read_timestamp = asf_read_pts,
.flags = AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH,
.priv_class = &asf_class,
};
