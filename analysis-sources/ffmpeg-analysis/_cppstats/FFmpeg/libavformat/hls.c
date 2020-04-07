#include "libavformat/http.h"
#include "libavutil/avstring.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/time.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"
#include "id3v2.h"
#define INITIAL_BUFFER_SIZE 32768
#define MAX_FIELD_LEN 64
#define MAX_CHARACTERISTICS_LEN 512
#define MPEG_TIME_BASE 90000
#define MPEG_TIME_BASE_Q (AVRational){1, MPEG_TIME_BASE}
enum KeyType {
KEY_NONE,
KEY_AES_128,
KEY_SAMPLE_AES
};
struct segment {
int64_t duration;
int64_t url_offset;
int64_t size;
char *url;
char *key;
enum KeyType key_type;
uint8_t iv[16];
struct segment *init_section;
};
struct rendition;
enum PlaylistType {
PLS_TYPE_UNSPECIFIED,
PLS_TYPE_EVENT,
PLS_TYPE_VOD
};
struct playlist {
char url[MAX_URL_SIZE];
AVIOContext pb;
uint8_t* read_buffer;
AVIOContext *input;
int input_read_done;
AVIOContext *input_next;
int input_next_requested;
AVFormatContext *parent;
int index;
AVFormatContext *ctx;
AVPacket pkt;
int has_noheader_flag;
AVStream **main_streams;
int n_main_streams;
int finished;
enum PlaylistType type;
int64_t target_duration;
int start_seq_no;
int n_segments;
struct segment **segments;
int needed;
int broken;
int cur_seq_no;
int last_seq_no;
int m3u8_hold_counters;
int64_t cur_seg_offset;
int64_t last_load_time;
struct segment *cur_init_section;
uint8_t *init_sec_buf;
unsigned int init_sec_buf_size;
unsigned int init_sec_data_len;
unsigned int init_sec_buf_read_offset;
char key_url[MAX_URL_SIZE];
uint8_t key[16];
int is_id3_timestamped; 
int64_t id3_mpegts_timestamp; 
int64_t id3_offset; 
uint8_t* id3_buf; 
unsigned int id3_buf_size;
AVDictionary *id3_initial; 
int id3_found; 
int id3_changed; 
ID3v2ExtraMeta *id3_deferred_extra; 
int64_t seek_timestamp;
int seek_flags;
int seek_stream_index; 
int n_renditions;
struct rendition **renditions;
int n_init_sections;
struct segment **init_sections;
};
struct rendition {
enum AVMediaType type;
struct playlist *playlist;
char group_id[MAX_FIELD_LEN];
char language[MAX_FIELD_LEN];
char name[MAX_FIELD_LEN];
int disposition;
};
struct variant {
int bandwidth;
int n_playlists;
struct playlist **playlists;
char audio_group[MAX_FIELD_LEN];
char video_group[MAX_FIELD_LEN];
char subtitles_group[MAX_FIELD_LEN];
};
typedef struct HLSContext {
AVClass *class;
AVFormatContext *ctx;
int n_variants;
struct variant **variants;
int n_playlists;
struct playlist **playlists;
int n_renditions;
struct rendition **renditions;
int cur_seq_no;
int m3u8_hold_counters;
int live_start_index;
int first_packet;
int64_t first_timestamp;
int64_t cur_timestamp;
AVIOInterruptCB *interrupt_callback;
AVDictionary *avio_opts;
char *allowed_extensions;
int max_reload;
int http_persistent;
int http_multiple;
int http_seekable;
AVIOContext *playlist_pb;
} HLSContext;
static void free_segment_dynarray(struct segment **segments, int n_segments)
{
int i;
for (i = 0; i < n_segments; i++) {
av_freep(&segments[i]->key);
av_freep(&segments[i]->url);
av_freep(&segments[i]);
}
}
static void free_segment_list(struct playlist *pls)
{
free_segment_dynarray(pls->segments, pls->n_segments);
av_freep(&pls->segments);
pls->n_segments = 0;
}
static void free_init_section_list(struct playlist *pls)
{
int i;
for (i = 0; i < pls->n_init_sections; i++) {
av_freep(&pls->init_sections[i]->url);
av_freep(&pls->init_sections[i]);
}
av_freep(&pls->init_sections);
pls->n_init_sections = 0;
}
static void free_playlist_list(HLSContext *c)
{
int i;
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
free_segment_list(pls);
free_init_section_list(pls);
av_freep(&pls->main_streams);
av_freep(&pls->renditions);
av_freep(&pls->id3_buf);
av_dict_free(&pls->id3_initial);
ff_id3v2_free_extra_meta(&pls->id3_deferred_extra);
av_freep(&pls->init_sec_buf);
av_packet_unref(&pls->pkt);
av_freep(&pls->pb.buffer);
ff_format_io_close(c->ctx, &pls->input);
pls->input_read_done = 0;
ff_format_io_close(c->ctx, &pls->input_next);
pls->input_next_requested = 0;
if (pls->ctx) {
pls->ctx->pb = NULL;
avformat_close_input(&pls->ctx);
}
av_free(pls);
}
av_freep(&c->playlists);
c->n_playlists = 0;
}
static void free_variant_list(HLSContext *c)
{
int i;
for (i = 0; i < c->n_variants; i++) {
struct variant *var = c->variants[i];
av_freep(&var->playlists);
av_free(var);
}
av_freep(&c->variants);
c->n_variants = 0;
}
static void free_rendition_list(HLSContext *c)
{
int i;
for (i = 0; i < c->n_renditions; i++)
av_freep(&c->renditions[i]);
av_freep(&c->renditions);
c->n_renditions = 0;
}
static void reset_packet(AVPacket *pkt)
{
av_init_packet(pkt);
pkt->data = NULL;
}
static struct playlist *new_playlist(HLSContext *c, const char *url,
const char *base)
{
struct playlist *pls = av_mallocz(sizeof(struct playlist));
if (!pls)
return NULL;
reset_packet(&pls->pkt);
ff_make_absolute_url(pls->url, sizeof(pls->url), base, url);
pls->seek_timestamp = AV_NOPTS_VALUE;
pls->is_id3_timestamped = -1;
pls->id3_mpegts_timestamp = AV_NOPTS_VALUE;
dynarray_add(&c->playlists, &c->n_playlists, pls);
return pls;
}
struct variant_info {
char bandwidth[20];
char audio[MAX_FIELD_LEN];
char video[MAX_FIELD_LEN];
char subtitles[MAX_FIELD_LEN];
};
static struct variant *new_variant(HLSContext *c, struct variant_info *info,
const char *url, const char *base)
{
struct variant *var;
struct playlist *pls;
pls = new_playlist(c, url, base);
if (!pls)
return NULL;
var = av_mallocz(sizeof(struct variant));
if (!var)
return NULL;
if (info) {
var->bandwidth = atoi(info->bandwidth);
strcpy(var->audio_group, info->audio);
strcpy(var->video_group, info->video);
strcpy(var->subtitles_group, info->subtitles);
}
dynarray_add(&c->variants, &c->n_variants, var);
dynarray_add(&var->playlists, &var->n_playlists, pls);
return var;
}
static void handle_variant_args(struct variant_info *info, const char *key,
int key_len, char **dest, int *dest_len)
{
if (!strncmp(key, "BANDWIDTH=", key_len)) {
*dest = info->bandwidth;
*dest_len = sizeof(info->bandwidth);
} else if (!strncmp(key, "AUDIO=", key_len)) {
*dest = info->audio;
*dest_len = sizeof(info->audio);
} else if (!strncmp(key, "VIDEO=", key_len)) {
*dest = info->video;
*dest_len = sizeof(info->video);
} else if (!strncmp(key, "SUBTITLES=", key_len)) {
*dest = info->subtitles;
*dest_len = sizeof(info->subtitles);
}
}
struct key_info {
char uri[MAX_URL_SIZE];
char method[11];
char iv[35];
};
static void handle_key_args(struct key_info *info, const char *key,
int key_len, char **dest, int *dest_len)
{
if (!strncmp(key, "METHOD=", key_len)) {
*dest = info->method;
*dest_len = sizeof(info->method);
} else if (!strncmp(key, "URI=", key_len)) {
*dest = info->uri;
*dest_len = sizeof(info->uri);
} else if (!strncmp(key, "IV=", key_len)) {
*dest = info->iv;
*dest_len = sizeof(info->iv);
}
}
struct init_section_info {
char uri[MAX_URL_SIZE];
char byterange[32];
};
static struct segment *new_init_section(struct playlist *pls,
struct init_section_info *info,
const char *url_base)
{
struct segment *sec;
char tmp_str[MAX_URL_SIZE], *ptr = tmp_str;
if (!info->uri[0])
return NULL;
sec = av_mallocz(sizeof(*sec));
if (!sec)
return NULL;
if (!av_strncasecmp(info->uri, "data:", 5)) {
ptr = info->uri;
} else {
ff_make_absolute_url(tmp_str, sizeof(tmp_str), url_base, info->uri);
}
sec->url = av_strdup(ptr);
if (!sec->url) {
av_free(sec);
return NULL;
}
if (info->byterange[0]) {
sec->size = strtoll(info->byterange, NULL, 10);
ptr = strchr(info->byterange, '@');
if (ptr)
sec->url_offset = strtoll(ptr+1, NULL, 10);
} else {
sec->size = -1;
}
dynarray_add(&pls->init_sections, &pls->n_init_sections, sec);
return sec;
}
static void handle_init_section_args(struct init_section_info *info, const char *key,
int key_len, char **dest, int *dest_len)
{
if (!strncmp(key, "URI=", key_len)) {
*dest = info->uri;
*dest_len = sizeof(info->uri);
} else if (!strncmp(key, "BYTERANGE=", key_len)) {
*dest = info->byterange;
*dest_len = sizeof(info->byterange);
}
}
struct rendition_info {
char type[16];
char uri[MAX_URL_SIZE];
char group_id[MAX_FIELD_LEN];
char language[MAX_FIELD_LEN];
char assoc_language[MAX_FIELD_LEN];
char name[MAX_FIELD_LEN];
char defaultr[4];
char forced[4];
char characteristics[MAX_CHARACTERISTICS_LEN];
};
static struct rendition *new_rendition(HLSContext *c, struct rendition_info *info,
const char *url_base)
{
struct rendition *rend;
enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
char *characteristic;
char *chr_ptr;
char *saveptr;
if (!strcmp(info->type, "AUDIO"))
type = AVMEDIA_TYPE_AUDIO;
else if (!strcmp(info->type, "VIDEO"))
type = AVMEDIA_TYPE_VIDEO;
else if (!strcmp(info->type, "SUBTITLES"))
type = AVMEDIA_TYPE_SUBTITLE;
else if (!strcmp(info->type, "CLOSED-CAPTIONS"))
return NULL;
if (type == AVMEDIA_TYPE_UNKNOWN) {
av_log(c->ctx, AV_LOG_WARNING, "Can't support the type: %s\n", info->type);
return NULL;
}
if (type == AVMEDIA_TYPE_SUBTITLE && !info->uri[0]) {
av_log(c->ctx, AV_LOG_ERROR, "The URI tag is REQUIRED for subtitle.\n");
return NULL;
}
if (c->ctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL)
if (type == AVMEDIA_TYPE_SUBTITLE) {
av_log(c->ctx, AV_LOG_WARNING, "Can't support the subtitle(uri: %s)\n", info->uri);
return NULL;
}
rend = av_mallocz(sizeof(struct rendition));
if (!rend)
return NULL;
dynarray_add(&c->renditions, &c->n_renditions, rend);
rend->type = type;
strcpy(rend->group_id, info->group_id);
strcpy(rend->language, info->language);
strcpy(rend->name, info->name);
if (info->uri[0]) {
rend->playlist = new_playlist(c, info->uri, url_base);
if (rend->playlist)
dynarray_add(&rend->playlist->renditions,
&rend->playlist->n_renditions, rend);
}
if (info->assoc_language[0]) {
int langlen = strlen(rend->language);
if (langlen < sizeof(rend->language) - 3) {
rend->language[langlen] = ',';
strncpy(rend->language + langlen + 1, info->assoc_language,
sizeof(rend->language) - langlen - 2);
}
}
if (!strcmp(info->defaultr, "YES"))
rend->disposition |= AV_DISPOSITION_DEFAULT;
if (!strcmp(info->forced, "YES"))
rend->disposition |= AV_DISPOSITION_FORCED;
chr_ptr = info->characteristics;
while ((characteristic = av_strtok(chr_ptr, ",", &saveptr))) {
if (!strcmp(characteristic, "public.accessibility.describes-music-and-sound"))
rend->disposition |= AV_DISPOSITION_HEARING_IMPAIRED;
else if (!strcmp(characteristic, "public.accessibility.describes-video"))
rend->disposition |= AV_DISPOSITION_VISUAL_IMPAIRED;
chr_ptr = NULL;
}
return rend;
}
static void handle_rendition_args(struct rendition_info *info, const char *key,
int key_len, char **dest, int *dest_len)
{
if (!strncmp(key, "TYPE=", key_len)) {
*dest = info->type;
*dest_len = sizeof(info->type);
} else if (!strncmp(key, "URI=", key_len)) {
*dest = info->uri;
*dest_len = sizeof(info->uri);
} else if (!strncmp(key, "GROUP-ID=", key_len)) {
*dest = info->group_id;
*dest_len = sizeof(info->group_id);
} else if (!strncmp(key, "LANGUAGE=", key_len)) {
*dest = info->language;
*dest_len = sizeof(info->language);
} else if (!strncmp(key, "ASSOC-LANGUAGE=", key_len)) {
*dest = info->assoc_language;
*dest_len = sizeof(info->assoc_language);
} else if (!strncmp(key, "NAME=", key_len)) {
*dest = info->name;
*dest_len = sizeof(info->name);
} else if (!strncmp(key, "DEFAULT=", key_len)) {
*dest = info->defaultr;
*dest_len = sizeof(info->defaultr);
} else if (!strncmp(key, "FORCED=", key_len)) {
*dest = info->forced;
*dest_len = sizeof(info->forced);
} else if (!strncmp(key, "CHARACTERISTICS=", key_len)) {
*dest = info->characteristics;
*dest_len = sizeof(info->characteristics);
}
}
static int ensure_playlist(HLSContext *c, struct playlist **pls, const char *url)
{
if (*pls)
return 0;
if (!new_variant(c, NULL, url, NULL))
return AVERROR(ENOMEM);
*pls = c->playlists[c->n_playlists - 1];
return 0;
}
static int open_url_keepalive(AVFormatContext *s, AVIOContext **pb,
const char *url, AVDictionary **options)
{
#if !CONFIG_HTTP_PROTOCOL
return AVERROR_PROTOCOL_NOT_FOUND;
#else
int ret;
URLContext *uc = ffio_geturlcontext(*pb);
av_assert0(uc);
(*pb)->eof_reached = 0;
ret = ff_http_do_new_request2(uc, url, options);
if (ret < 0) {
ff_format_io_close(s, pb);
}
return ret;
#endif
}
static int open_url(AVFormatContext *s, AVIOContext **pb, const char *url,
AVDictionary *opts, AVDictionary *opts2, int *is_http_out)
{
HLSContext *c = s->priv_data;
AVDictionary *tmp = NULL;
const char *proto_name = NULL;
int ret;
int is_http = 0;
if (av_strstart(url, "crypto", NULL)) {
if (url[6] == '+' || url[6] == ':')
proto_name = avio_find_protocol_name(url + 7);
} else if (av_strstart(url, "data", NULL)) {
if (url[4] == '+' || url[4] == ':')
proto_name = avio_find_protocol_name(url + 5);
}
if (!proto_name)
proto_name = avio_find_protocol_name(url);
if (!proto_name)
return AVERROR_INVALIDDATA;
if (av_strstart(proto_name, "file", NULL)) {
if (strcmp(c->allowed_extensions, "ALL") && !av_match_ext(url, c->allowed_extensions)) {
av_log(s, AV_LOG_ERROR,
"Filename extension of \'%s\' is not a common multimedia extension, blocked for security reasons.\n"
"If you wish to override this adjust allowed_extensions, you can set it to \'ALL\' to allow all\n",
url);
return AVERROR_INVALIDDATA;
}
} else if (av_strstart(proto_name, "http", NULL)) {
is_http = 1;
} else if (av_strstart(proto_name, "data", NULL)) {
;
} else
return AVERROR_INVALIDDATA;
if (!strncmp(proto_name, url, strlen(proto_name)) && url[strlen(proto_name)] == ':')
;
else if (av_strstart(url, "crypto", NULL) && !strncmp(proto_name, url + 7, strlen(proto_name)) && url[7 + strlen(proto_name)] == ':')
;
else if (av_strstart(url, "data", NULL) && !strncmp(proto_name, url + 5, strlen(proto_name)) && url[5 + strlen(proto_name)] == ':')
;
else if (strcmp(proto_name, "file") || !strncmp(url, "file,", 5))
return AVERROR_INVALIDDATA;
av_dict_copy(&tmp, opts, 0);
av_dict_copy(&tmp, opts2, 0);
if (is_http && c->http_persistent && *pb) {
ret = open_url_keepalive(c->ctx, pb, url, &tmp);
if (ret == AVERROR_EXIT) {
av_dict_free(&tmp);
return ret;
} else if (ret < 0) {
if (ret != AVERROR_EOF)
av_log(s, AV_LOG_WARNING,
"keepalive request failed for '%s' with error: '%s' when opening url, retrying with new connection\n",
url, av_err2str(ret));
ret = s->io_open(s, pb, url, AVIO_FLAG_READ, &tmp);
}
} else {
ret = s->io_open(s, pb, url, AVIO_FLAG_READ, &tmp);
}
if (ret >= 0) {
char *new_cookies = NULL;
if (!(s->flags & AVFMT_FLAG_CUSTOM_IO))
av_opt_get(*pb, "cookies", AV_OPT_SEARCH_CHILDREN, (uint8_t**)&new_cookies);
if (new_cookies)
av_dict_set(&opts, "cookies", new_cookies, AV_DICT_DONT_STRDUP_VAL);
}
av_dict_free(&tmp);
if (is_http_out)
*is_http_out = is_http;
return ret;
}
static int parse_playlist(HLSContext *c, const char *url,
struct playlist *pls, AVIOContext *in)
{
int ret = 0, is_segment = 0, is_variant = 0;
int64_t duration = 0;
enum KeyType key_type = KEY_NONE;
uint8_t iv[16] = "";
int has_iv = 0;
char key[MAX_URL_SIZE] = "";
char line[MAX_URL_SIZE];
const char *ptr;
int close_in = 0;
int64_t seg_offset = 0;
int64_t seg_size = -1;
uint8_t *new_url = NULL;
struct variant_info variant_info;
char tmp_str[MAX_URL_SIZE];
struct segment *cur_init_section = NULL;
int is_http = av_strstart(url, "http", NULL);
struct segment **prev_segments = NULL;
int prev_n_segments = 0;
int prev_start_seq_no = -1;
if (is_http && !in && c->http_persistent && c->playlist_pb) {
in = c->playlist_pb;
ret = open_url_keepalive(c->ctx, &c->playlist_pb, url, NULL);
if (ret == AVERROR_EXIT) {
return ret;
} else if (ret < 0) {
if (ret != AVERROR_EOF)
av_log(c->ctx, AV_LOG_WARNING,
"keepalive request failed for '%s' with error: '%s' when parsing playlist\n",
url, av_err2str(ret));
in = NULL;
}
}
if (!in) {
AVDictionary *opts = NULL;
av_dict_copy(&opts, c->avio_opts, 0);
if (c->http_persistent)
av_dict_set(&opts, "multiple_requests", "1", 0);
ret = c->ctx->io_open(c->ctx, &in, url, AVIO_FLAG_READ, &opts);
av_dict_free(&opts);
if (ret < 0)
return ret;
if (is_http && c->http_persistent)
c->playlist_pb = in;
else
close_in = 1;
}
if (av_opt_get(in, "location", AV_OPT_SEARCH_CHILDREN, &new_url) >= 0)
url = new_url;
ff_get_chomp_line(in, line, sizeof(line));
if (strcmp(line, "#EXTM3U")) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
if (pls) {
prev_start_seq_no = pls->start_seq_no;
prev_segments = pls->segments;
prev_n_segments = pls->n_segments;
pls->segments = NULL;
pls->n_segments = 0;
pls->finished = 0;
pls->type = PLS_TYPE_UNSPECIFIED;
}
while (!avio_feof(in)) {
ff_get_chomp_line(in, line, sizeof(line));
if (av_strstart(line, "#EXT-X-STREAM-INF:", &ptr)) {
is_variant = 1;
memset(&variant_info, 0, sizeof(variant_info));
ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_variant_args,
&variant_info);
} else if (av_strstart(line, "#EXT-X-KEY:", &ptr)) {
struct key_info info = {{0}};
ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_key_args,
&info);
key_type = KEY_NONE;
has_iv = 0;
if (!strcmp(info.method, "AES-128"))
key_type = KEY_AES_128;
if (!strcmp(info.method, "SAMPLE-AES"))
key_type = KEY_SAMPLE_AES;
if (!strncmp(info.iv, "0x", 2) || !strncmp(info.iv, "0X", 2)) {
ff_hex_to_data(iv, info.iv + 2);
has_iv = 1;
}
av_strlcpy(key, info.uri, sizeof(key));
} else if (av_strstart(line, "#EXT-X-MEDIA:", &ptr)) {
struct rendition_info info = {{0}};
ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_rendition_args,
&info);
new_rendition(c, &info, url);
} else if (av_strstart(line, "#EXT-X-TARGETDURATION:", &ptr)) {
ret = ensure_playlist(c, &pls, url);
if (ret < 0)
goto fail;
pls->target_duration = strtoll(ptr, NULL, 10) * AV_TIME_BASE;
} else if (av_strstart(line, "#EXT-X-MEDIA-SEQUENCE:", &ptr)) {
ret = ensure_playlist(c, &pls, url);
if (ret < 0)
goto fail;
pls->start_seq_no = atoi(ptr);
} else if (av_strstart(line, "#EXT-X-PLAYLIST-TYPE:", &ptr)) {
ret = ensure_playlist(c, &pls, url);
if (ret < 0)
goto fail;
if (!strcmp(ptr, "EVENT"))
pls->type = PLS_TYPE_EVENT;
else if (!strcmp(ptr, "VOD"))
pls->type = PLS_TYPE_VOD;
} else if (av_strstart(line, "#EXT-X-MAP:", &ptr)) {
struct init_section_info info = {{0}};
ret = ensure_playlist(c, &pls, url);
if (ret < 0)
goto fail;
ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_init_section_args,
&info);
cur_init_section = new_init_section(pls, &info, url);
cur_init_section->key_type = key_type;
if (has_iv) {
memcpy(cur_init_section->iv, iv, sizeof(iv));
} else {
int seq = pls->start_seq_no + pls->n_segments;
memset(cur_init_section->iv, 0, sizeof(cur_init_section->iv));
AV_WB32(cur_init_section->iv + 12, seq);
}
if (key_type != KEY_NONE) {
ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, key);
cur_init_section->key = av_strdup(tmp_str);
if (!cur_init_section->key) {
av_free(cur_init_section);
ret = AVERROR(ENOMEM);
goto fail;
}
} else {
cur_init_section->key = NULL;
}
} else if (av_strstart(line, "#EXT-X-ENDLIST", &ptr)) {
if (pls)
pls->finished = 1;
} else if (av_strstart(line, "#EXTINF:", &ptr)) {
is_segment = 1;
duration = atof(ptr) * AV_TIME_BASE;
} else if (av_strstart(line, "#EXT-X-BYTERANGE:", &ptr)) {
seg_size = strtoll(ptr, NULL, 10);
ptr = strchr(ptr, '@');
if (ptr)
seg_offset = strtoll(ptr+1, NULL, 10);
} else if (av_strstart(line, "#", NULL)) {
av_log(c->ctx, AV_LOG_INFO, "Skip ('%s')\n", line);
continue;
} else if (line[0]) {
if (is_variant) {
if (!new_variant(c, &variant_info, line, url)) {
ret = AVERROR(ENOMEM);
goto fail;
}
is_variant = 0;
}
if (is_segment) {
struct segment *seg;
ret = ensure_playlist(c, &pls, url);
if (ret < 0)
goto fail;
seg = av_malloc(sizeof(struct segment));
if (!seg) {
ret = AVERROR(ENOMEM);
goto fail;
}
seg->duration = duration;
seg->key_type = key_type;
if (has_iv) {
memcpy(seg->iv, iv, sizeof(iv));
} else {
int seq = pls->start_seq_no + pls->n_segments;
memset(seg->iv, 0, sizeof(seg->iv));
AV_WB32(seg->iv + 12, seq);
}
if (key_type != KEY_NONE) {
ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, key);
seg->key = av_strdup(tmp_str);
if (!seg->key) {
av_free(seg);
ret = AVERROR(ENOMEM);
goto fail;
}
} else {
seg->key = NULL;
}
ff_make_absolute_url(tmp_str, sizeof(tmp_str), url, line);
seg->url = av_strdup(tmp_str);
if (!seg->url) {
av_free(seg->key);
av_free(seg);
ret = AVERROR(ENOMEM);
goto fail;
}
dynarray_add(&pls->segments, &pls->n_segments, seg);
is_segment = 0;
seg->size = seg_size;
if (seg_size >= 0) {
seg->url_offset = seg_offset;
seg_offset += seg_size;
seg_size = -1;
} else {
seg->url_offset = 0;
seg_offset = 0;
}
seg->init_section = cur_init_section;
}
}
}
if (prev_segments) {
if (pls->start_seq_no > prev_start_seq_no && c->first_timestamp != AV_NOPTS_VALUE) {
int64_t prev_timestamp = c->first_timestamp;
int i, diff = pls->start_seq_no - prev_start_seq_no;
for (i = 0; i < prev_n_segments && i < diff; i++) {
c->first_timestamp += prev_segments[i]->duration;
}
av_log(c->ctx, AV_LOG_DEBUG, "Media sequence change (%d -> %d)"
" reflected in first_timestamp: %"PRId64" -> %"PRId64"\n",
prev_start_seq_no, pls->start_seq_no,
prev_timestamp, c->first_timestamp);
} else if (pls->start_seq_no < prev_start_seq_no) {
av_log(c->ctx, AV_LOG_WARNING, "Media sequence changed unexpectedly: %d -> %d\n",
prev_start_seq_no, pls->start_seq_no);
}
free_segment_dynarray(prev_segments, prev_n_segments);
av_freep(&prev_segments);
}
if (pls)
pls->last_load_time = av_gettime_relative();
fail:
av_free(new_url);
if (close_in)
ff_format_io_close(c->ctx, &in);
c->ctx->ctx_flags = c->ctx->ctx_flags & ~(unsigned)AVFMTCTX_UNSEEKABLE;
if (!c->n_variants || !c->variants[0]->n_playlists ||
!(c->variants[0]->playlists[0]->finished ||
c->variants[0]->playlists[0]->type == PLS_TYPE_EVENT))
c->ctx->ctx_flags |= AVFMTCTX_UNSEEKABLE;
return ret;
}
static struct segment *current_segment(struct playlist *pls)
{
return pls->segments[pls->cur_seq_no - pls->start_seq_no];
}
static struct segment *next_segment(struct playlist *pls)
{
int n = pls->cur_seq_no - pls->start_seq_no + 1;
if (n >= pls->n_segments)
return NULL;
return pls->segments[n];
}
static int read_from_url(struct playlist *pls, struct segment *seg,
uint8_t *buf, int buf_size)
{
int ret;
if (seg->size >= 0)
buf_size = FFMIN(buf_size, seg->size - pls->cur_seg_offset);
ret = avio_read(pls->input, buf, buf_size);
if (ret > 0)
pls->cur_seg_offset += ret;
return ret;
}
static void parse_id3(AVFormatContext *s, AVIOContext *pb,
AVDictionary **metadata, int64_t *dts,
ID3v2ExtraMetaAPIC **apic, ID3v2ExtraMeta **extra_meta)
{
static const char id3_priv_owner_ts[] = "com.apple.streaming.transportStreamTimestamp";
ID3v2ExtraMeta *meta;
ff_id3v2_read_dict(pb, metadata, ID3v2_DEFAULT_MAGIC, extra_meta);
for (meta = *extra_meta; meta; meta = meta->next) {
if (!strcmp(meta->tag, "PRIV")) {
ID3v2ExtraMetaPRIV *priv = meta->data;
if (priv->datasize == 8 && !strcmp(priv->owner, id3_priv_owner_ts)) {
int64_t ts = AV_RB64(priv->data);
av_log(s, AV_LOG_DEBUG, "HLS ID3 audio timestamp %"PRId64"\n", ts);
if ((ts & ~((1ULL << 33) - 1)) == 0)
*dts = ts;
else
av_log(s, AV_LOG_ERROR, "Invalid HLS ID3 audio timestamp %"PRId64"\n", ts);
}
} else if (!strcmp(meta->tag, "APIC") && apic)
*apic = meta->data;
}
}
static int id3_has_changed_values(struct playlist *pls, AVDictionary *metadata,
ID3v2ExtraMetaAPIC *apic)
{
AVDictionaryEntry *entry = NULL;
AVDictionaryEntry *oldentry;
while ((entry = av_dict_get(metadata, "", entry, AV_DICT_IGNORE_SUFFIX))) {
oldentry = av_dict_get(pls->id3_initial, entry->key, NULL, AV_DICT_MATCH_CASE);
if (!oldentry || strcmp(oldentry->value, entry->value) != 0)
return 1;
}
if (apic && (pls->ctx->nb_streams != 2 || !pls->ctx->streams[1]->attached_pic.data))
return 1;
if (apic) {
int size = pls->ctx->streams[1]->attached_pic.size;
if (size != apic->buf->size - AV_INPUT_BUFFER_PADDING_SIZE)
return 1;
if (memcmp(apic->buf->data, pls->ctx->streams[1]->attached_pic.data, size) != 0)
return 1;
}
return 0;
}
static void handle_id3(AVIOContext *pb, struct playlist *pls)
{
AVDictionary *metadata = NULL;
ID3v2ExtraMetaAPIC *apic = NULL;
ID3v2ExtraMeta *extra_meta = NULL;
int64_t timestamp = AV_NOPTS_VALUE;
parse_id3(pls->ctx, pb, &metadata, &timestamp, &apic, &extra_meta);
if (timestamp != AV_NOPTS_VALUE) {
pls->id3_mpegts_timestamp = timestamp;
pls->id3_offset = 0;
}
if (!pls->id3_found) {
av_assert0(!pls->id3_deferred_extra);
pls->id3_found = 1;
if (pls->ctx->nb_streams)
ff_id3v2_parse_apic(pls->ctx, &extra_meta);
else
pls->id3_deferred_extra = extra_meta;
ff_id3v2_parse_priv_dict(&metadata, &extra_meta);
av_dict_copy(&pls->ctx->metadata, metadata, 0);
pls->id3_initial = metadata;
} else {
if (!pls->id3_changed && id3_has_changed_values(pls, metadata, apic)) {
avpriv_report_missing_feature(pls->parent, "Changing ID3 metadata in HLS audio elementary stream");
pls->id3_changed = 1;
}
av_dict_free(&metadata);
}
if (!pls->id3_deferred_extra)
ff_id3v2_free_extra_meta(&extra_meta);
}
static void intercept_id3(struct playlist *pls, uint8_t *buf,
int buf_size, int *len)
{
int bytes;
int id3_buf_pos = 0;
int fill_buf = 0;
struct segment *seg = current_segment(pls);
while (1) {
if (*len < ID3v2_HEADER_SIZE && buf_size >= ID3v2_HEADER_SIZE) {
bytes = read_from_url(pls, seg, buf + *len, ID3v2_HEADER_SIZE - *len);
if (bytes > 0) {
if (bytes == ID3v2_HEADER_SIZE - *len)
fill_buf = 1;
*len += bytes;
} else if (*len <= 0) {
*len = bytes;
fill_buf = 0;
}
}
if (*len < ID3v2_HEADER_SIZE)
break;
if (ff_id3v2_match(buf, ID3v2_DEFAULT_MAGIC)) {
int64_t maxsize = seg->size >= 0 ? seg->size : 1024*1024;
int taglen = ff_id3v2_tag_len(buf);
int tag_got_bytes = FFMIN(taglen, *len);
int remaining = taglen - tag_got_bytes;
if (taglen > maxsize) {
av_log(pls->parent, AV_LOG_ERROR, "Too large HLS ID3 tag (%d > %"PRId64" bytes)\n",
taglen, maxsize);
break;
}
pls->id3_buf = av_fast_realloc(pls->id3_buf, &pls->id3_buf_size, id3_buf_pos + taglen);
if (!pls->id3_buf)
break;
memcpy(pls->id3_buf + id3_buf_pos, buf, tag_got_bytes);
id3_buf_pos += tag_got_bytes;
*len -= tag_got_bytes;
memmove(buf, buf + tag_got_bytes, *len);
av_log(pls->parent, AV_LOG_DEBUG, "Stripped %d HLS ID3 bytes\n", tag_got_bytes);
if (remaining > 0) {
if (read_from_url(pls, seg, pls->id3_buf + id3_buf_pos, remaining) != remaining)
break;
id3_buf_pos += remaining;
av_log(pls->parent, AV_LOG_DEBUG, "Stripped additional %d HLS ID3 bytes\n", remaining);
}
} else {
break;
}
}
if (*len >= 0 && (fill_buf || *len == 0)) {
bytes = read_from_url(pls, seg, buf + *len, buf_size - *len);
if (bytes >= 0)
*len += bytes;
else if (*len == 0)
*len = bytes;
}
if (pls->id3_buf) {
AVIOContext id3ioctx;
ffio_init_context(&id3ioctx, pls->id3_buf, id3_buf_pos, 0, NULL, NULL, NULL, NULL);
handle_id3(&id3ioctx, pls);
}
if (pls->is_id3_timestamped == -1)
pls->is_id3_timestamped = (pls->id3_mpegts_timestamp != AV_NOPTS_VALUE);
}
static int open_input(HLSContext *c, struct playlist *pls, struct segment *seg, AVIOContext **in)
{
AVDictionary *opts = NULL;
int ret;
int is_http = 0;
if (c->http_persistent)
av_dict_set(&opts, "multiple_requests", "1", 0);
if (seg->size >= 0) {
av_dict_set_int(&opts, "offset", seg->url_offset, 0);
av_dict_set_int(&opts, "end_offset", seg->url_offset + seg->size, 0);
}
av_log(pls->parent, AV_LOG_VERBOSE, "HLS request for url '%s', offset %"PRId64", playlist %d\n",
seg->url, seg->url_offset, pls->index);
if (seg->key_type == KEY_NONE) {
ret = open_url(pls->parent, in, seg->url, c->avio_opts, opts, &is_http);
} else if (seg->key_type == KEY_AES_128) {
char iv[33], key[33], url[MAX_URL_SIZE];
if (strcmp(seg->key, pls->key_url)) {
AVIOContext *pb = NULL;
if (open_url(pls->parent, &pb, seg->key, c->avio_opts, opts, NULL) == 0) {
ret = avio_read(pb, pls->key, sizeof(pls->key));
if (ret != sizeof(pls->key)) {
av_log(pls->parent, AV_LOG_ERROR, "Unable to read key file %s\n",
seg->key);
}
ff_format_io_close(pls->parent, &pb);
} else {
av_log(pls->parent, AV_LOG_ERROR, "Unable to open key file %s\n",
seg->key);
}
av_strlcpy(pls->key_url, seg->key, sizeof(pls->key_url));
}
ff_data_to_hex(iv, seg->iv, sizeof(seg->iv), 0);
ff_data_to_hex(key, pls->key, sizeof(pls->key), 0);
iv[32] = key[32] = '\0';
if (strstr(seg->url, "://"))
snprintf(url, sizeof(url), "crypto+%s", seg->url);
else
snprintf(url, sizeof(url), "crypto:%s", seg->url);
av_dict_set(&opts, "key", key, 0);
av_dict_set(&opts, "iv", iv, 0);
ret = open_url(pls->parent, in, url, c->avio_opts, opts, &is_http);
if (ret < 0) {
goto cleanup;
}
ret = 0;
} else if (seg->key_type == KEY_SAMPLE_AES) {
av_log(pls->parent, AV_LOG_ERROR,
"SAMPLE-AES encryption is not supported yet\n");
ret = AVERROR_PATCHWELCOME;
}
else
ret = AVERROR(ENOSYS);
if (ret == 0 && !is_http && seg->key_type == KEY_NONE && seg->url_offset) {
int64_t seekret = avio_seek(*in, seg->url_offset, SEEK_SET);
if (seekret < 0) {
av_log(pls->parent, AV_LOG_ERROR, "Unable to seek to offset %"PRId64" of HLS segment '%s'\n", seg->url_offset, seg->url);
ret = seekret;
ff_format_io_close(pls->parent, in);
}
}
cleanup:
av_dict_free(&opts);
pls->cur_seg_offset = 0;
return ret;
}
static int update_init_section(struct playlist *pls, struct segment *seg)
{
static const int max_init_section_size = 1024*1024;
HLSContext *c = pls->parent->priv_data;
int64_t sec_size;
int64_t urlsize;
int ret;
if (seg->init_section == pls->cur_init_section)
return 0;
pls->cur_init_section = NULL;
if (!seg->init_section)
return 0;
ret = open_input(c, pls, seg->init_section, &pls->input);
if (ret < 0) {
av_log(pls->parent, AV_LOG_WARNING,
"Failed to open an initialization section in playlist %d\n",
pls->index);
return ret;
}
if (seg->init_section->size >= 0)
sec_size = seg->init_section->size;
else if ((urlsize = avio_size(pls->input)) >= 0)
sec_size = urlsize;
else
sec_size = max_init_section_size;
av_log(pls->parent, AV_LOG_DEBUG,
"Downloading an initialization section of size %"PRId64"\n",
sec_size);
sec_size = FFMIN(sec_size, max_init_section_size);
av_fast_malloc(&pls->init_sec_buf, &pls->init_sec_buf_size, sec_size);
ret = read_from_url(pls, seg->init_section, pls->init_sec_buf,
pls->init_sec_buf_size);
ff_format_io_close(pls->parent, &pls->input);
if (ret < 0)
return ret;
pls->cur_init_section = seg->init_section;
pls->init_sec_data_len = ret;
pls->init_sec_buf_read_offset = 0;
pls->is_id3_timestamped = 0;
return 0;
}
static int64_t default_reload_interval(struct playlist *pls)
{
return pls->n_segments > 0 ?
pls->segments[pls->n_segments - 1]->duration :
pls->target_duration;
}
static int playlist_needed(struct playlist *pls)
{
AVFormatContext *s = pls->parent;
int i, j;
int stream_needed = 0;
int first_st;
if (!pls->ctx || !pls->n_main_streams)
return 1;
for (i = 0; i < pls->n_main_streams; i++) {
if (pls->main_streams[i]->discard < AVDISCARD_ALL) {
stream_needed = 1;
break;
}
}
if (!stream_needed)
return 0;
first_st = pls->main_streams[0]->index;
for (i = 0; i < s->nb_programs; i++) {
AVProgram *program = s->programs[i];
if (program->discard < AVDISCARD_ALL) {
for (j = 0; j < program->nb_stream_indexes; j++) {
if (program->stream_index[j] == first_st) {
return 1;
}
}
}
}
return 0;
}
static int read_data(void *opaque, uint8_t *buf, int buf_size)
{
struct playlist *v = opaque;
HLSContext *c = v->parent->priv_data;
int ret;
int just_opened = 0;
int reload_count = 0;
struct segment *seg;
restart:
if (!v->needed)
return AVERROR_EOF;
if (!v->input || (c->http_persistent && v->input_read_done)) {
int64_t reload_interval;
v->needed = playlist_needed(v);
if (!v->needed) {
av_log(v->parent, AV_LOG_INFO, "No longer receiving playlist %d ('%s')\n",
v->index, v->url);
return AVERROR_EOF;
}
reload_interval = default_reload_interval(v);
reload:
reload_count++;
if (reload_count > c->max_reload)
return AVERROR_EOF;
if (!v->finished &&
av_gettime_relative() - v->last_load_time >= reload_interval) {
if ((ret = parse_playlist(c, v->url, v, NULL)) < 0) {
if (ret != AVERROR_EXIT)
av_log(v->parent, AV_LOG_WARNING, "Failed to reload playlist %d\n",
v->index);
return ret;
}
reload_interval = v->target_duration / 2;
}
if (v->cur_seq_no < v->start_seq_no) {
av_log(v->parent, AV_LOG_WARNING,
"skipping %d segments ahead, expired from playlists\n",
v->start_seq_no - v->cur_seq_no);
v->cur_seq_no = v->start_seq_no;
}
if (v->cur_seq_no > v->last_seq_no) {
v->last_seq_no = v->cur_seq_no;
v->m3u8_hold_counters = 0;
} else if (v->last_seq_no == v->cur_seq_no) {
v->m3u8_hold_counters++;
if (v->m3u8_hold_counters >= c->m3u8_hold_counters) {
return AVERROR_EOF;
}
} else {
av_log(v->parent, AV_LOG_WARNING, "maybe the m3u8 list sequence have been wraped.\n");
}
if (v->cur_seq_no >= v->start_seq_no + v->n_segments) {
if (v->finished)
return AVERROR_EOF;
while (av_gettime_relative() - v->last_load_time < reload_interval) {
if (ff_check_interrupt(c->interrupt_callback))
return AVERROR_EXIT;
av_usleep(100*1000);
}
goto reload;
}
v->input_read_done = 0;
seg = current_segment(v);
ret = update_init_section(v, seg);
if (ret)
return ret;
if (c->http_multiple == 1 && v->input_next_requested) {
FFSWAP(AVIOContext *, v->input, v->input_next);
v->cur_seg_offset = 0;
v->input_next_requested = 0;
ret = 0;
} else {
ret = open_input(c, v, seg, &v->input);
}
if (ret < 0) {
if (ff_check_interrupt(c->interrupt_callback))
return AVERROR_EXIT;
av_log(v->parent, AV_LOG_WARNING, "Failed to open segment %d of playlist %d\n",
v->cur_seq_no,
v->index);
v->cur_seq_no += 1;
goto reload;
}
just_opened = 1;
}
if (c->http_multiple == -1) {
uint8_t *http_version_opt = NULL;
int r = av_opt_get(v->input, "http_version", AV_OPT_SEARCH_CHILDREN, &http_version_opt);
if (r >= 0) {
c->http_multiple = (!strncmp((const char *)http_version_opt, "1.1", 3) || !strncmp((const char *)http_version_opt, "2.0", 3));
av_freep(&http_version_opt);
}
}
seg = next_segment(v);
if (c->http_multiple == 1 && !v->input_next_requested &&
seg && seg->key_type == KEY_NONE && av_strstart(seg->url, "http", NULL)) {
ret = open_input(c, v, seg, &v->input_next);
if (ret < 0) {
if (ff_check_interrupt(c->interrupt_callback))
return AVERROR_EXIT;
av_log(v->parent, AV_LOG_WARNING, "Failed to open next segment %d of playlist %d\n",
v->cur_seq_no + 1,
v->index);
} else {
v->input_next_requested = 1;
}
}
if (v->init_sec_buf_read_offset < v->init_sec_data_len) {
int copy_size = FFMIN(v->init_sec_data_len - v->init_sec_buf_read_offset, buf_size);
memcpy(buf, v->init_sec_buf, copy_size);
v->init_sec_buf_read_offset += copy_size;
return copy_size;
}
seg = current_segment(v);
ret = read_from_url(v, seg, buf, buf_size);
if (ret > 0) {
if (just_opened && v->is_id3_timestamped != 0) {
intercept_id3(v, buf, buf_size, &ret);
}
return ret;
}
if (c->http_persistent &&
seg->key_type == KEY_NONE && av_strstart(seg->url, "http", NULL)) {
v->input_read_done = 1;
} else {
ff_format_io_close(v->parent, &v->input);
}
v->cur_seq_no++;
c->cur_seq_no = v->cur_seq_no;
goto restart;
}
static void add_renditions_to_variant(HLSContext *c, struct variant *var,
enum AVMediaType type, const char *group_id)
{
int i;
for (i = 0; i < c->n_renditions; i++) {
struct rendition *rend = c->renditions[i];
if (rend->type == type && !strcmp(rend->group_id, group_id)) {
if (rend->playlist)
dynarray_add(&var->playlists, &var->n_playlists, rend->playlist);
else
dynarray_add(&var->playlists[0]->renditions,
&var->playlists[0]->n_renditions,
rend);
}
}
}
static void add_metadata_from_renditions(AVFormatContext *s, struct playlist *pls,
enum AVMediaType type)
{
int rend_idx = 0;
int i;
for (i = 0; i < pls->n_main_streams; i++) {
AVStream *st = pls->main_streams[i];
if (st->codecpar->codec_type != type)
continue;
for (; rend_idx < pls->n_renditions; rend_idx++) {
struct rendition *rend = pls->renditions[rend_idx];
if (rend->type != type)
continue;
if (rend->language[0])
av_dict_set(&st->metadata, "language", rend->language, 0);
if (rend->name[0])
av_dict_set(&st->metadata, "comment", rend->name, 0);
st->disposition |= rend->disposition;
}
if (rend_idx >=pls->n_renditions)
break;
}
}
static int find_timestamp_in_playlist(HLSContext *c, struct playlist *pls,
int64_t timestamp, int *seq_no)
{
int i;
int64_t pos = c->first_timestamp == AV_NOPTS_VALUE ?
0 : c->first_timestamp;
if (timestamp < pos) {
*seq_no = pls->start_seq_no;
return 0;
}
for (i = 0; i < pls->n_segments; i++) {
int64_t diff = pos + pls->segments[i]->duration - timestamp;
if (diff > 0) {
*seq_no = pls->start_seq_no + i;
return 1;
}
pos += pls->segments[i]->duration;
}
*seq_no = pls->start_seq_no + pls->n_segments - 1;
return 0;
}
static int select_cur_seq_no(HLSContext *c, struct playlist *pls)
{
int seq_no;
if (!pls->finished && !c->first_packet &&
av_gettime_relative() - pls->last_load_time >= default_reload_interval(pls))
parse_playlist(c, pls->url, pls, NULL);
if (pls->finished && c->cur_timestamp != AV_NOPTS_VALUE) {
find_timestamp_in_playlist(c, pls, c->cur_timestamp, &seq_no);
return seq_no;
}
if (!pls->finished) {
if (!c->first_packet && 
c->cur_seq_no >= pls->start_seq_no &&
c->cur_seq_no < pls->start_seq_no + pls->n_segments)
return c->cur_seq_no;
if (c->live_start_index < 0)
return pls->start_seq_no + FFMAX(pls->n_segments + c->live_start_index, 0);
else
return pls->start_seq_no + FFMIN(c->live_start_index, pls->n_segments - 1);
}
return pls->start_seq_no;
}
static int save_avio_options(AVFormatContext *s)
{
HLSContext *c = s->priv_data;
static const char * const opts[] = {
"headers", "http_proxy", "user_agent", "cookies", "referer", "rw_timeout", "icy", NULL };
const char * const * opt = opts;
uint8_t *buf;
int ret = 0;
while (*opt) {
if (av_opt_get(s->pb, *opt, AV_OPT_SEARCH_CHILDREN | AV_OPT_ALLOW_NULL, &buf) >= 0) {
ret = av_dict_set(&c->avio_opts, *opt, buf,
AV_DICT_DONT_STRDUP_VAL);
if (ret < 0)
return ret;
}
opt++;
}
return ret;
}
static int nested_io_open(AVFormatContext *s, AVIOContext **pb, const char *url,
int flags, AVDictionary **opts)
{
av_log(s, AV_LOG_ERROR,
"A HLS playlist item '%s' referred to an external file '%s'. "
"Opening this file was forbidden for security reasons\n",
s->url, url);
return AVERROR(EPERM);
}
static void add_stream_to_programs(AVFormatContext *s, struct playlist *pls, AVStream *stream)
{
HLSContext *c = s->priv_data;
int i, j;
int bandwidth = -1;
for (i = 0; i < c->n_variants; i++) {
struct variant *v = c->variants[i];
for (j = 0; j < v->n_playlists; j++) {
if (v->playlists[j] != pls)
continue;
av_program_add_stream_index(s, i, stream->index);
if (bandwidth < 0)
bandwidth = v->bandwidth;
else if (bandwidth != v->bandwidth)
bandwidth = -1; 
}
}
if (bandwidth >= 0)
av_dict_set_int(&stream->metadata, "variant_bitrate", bandwidth, 0);
}
static int set_stream_info_from_input_stream(AVStream *st, struct playlist *pls, AVStream *ist)
{
int err;
err = avcodec_parameters_copy(st->codecpar, ist->codecpar);
if (err < 0)
return err;
if (pls->is_id3_timestamped) 
avpriv_set_pts_info(st, 33, 1, MPEG_TIME_BASE);
else
avpriv_set_pts_info(st, ist->pts_wrap_bits, ist->time_base.num, ist->time_base.den);
st->internal->need_context_update = 1;
return 0;
}
static int update_streams_from_subdemuxer(AVFormatContext *s, struct playlist *pls)
{
int err;
while (pls->n_main_streams < pls->ctx->nb_streams) {
int ist_idx = pls->n_main_streams;
AVStream *st = avformat_new_stream(s, NULL);
AVStream *ist = pls->ctx->streams[ist_idx];
if (!st)
return AVERROR(ENOMEM);
st->id = pls->index;
dynarray_add(&pls->main_streams, &pls->n_main_streams, st);
add_stream_to_programs(s, pls, st);
err = set_stream_info_from_input_stream(st, pls, ist);
if (err < 0)
return err;
}
return 0;
}
static void update_noheader_flag(AVFormatContext *s)
{
HLSContext *c = s->priv_data;
int flag_needed = 0;
int i;
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
if (pls->has_noheader_flag) {
flag_needed = 1;
break;
}
}
if (flag_needed)
s->ctx_flags |= AVFMTCTX_NOHEADER;
else
s->ctx_flags &= ~AVFMTCTX_NOHEADER;
}
static int hls_close(AVFormatContext *s)
{
HLSContext *c = s->priv_data;
free_playlist_list(c);
free_variant_list(c);
free_rendition_list(c);
av_dict_free(&c->avio_opts);
ff_format_io_close(c->ctx, &c->playlist_pb);
return 0;
}
static int hls_read_header(AVFormatContext *s)
{
HLSContext *c = s->priv_data;
int ret = 0, i;
int highest_cur_seq_no = 0;
c->ctx = s;
c->interrupt_callback = &s->interrupt_callback;
c->first_packet = 1;
c->first_timestamp = AV_NOPTS_VALUE;
c->cur_timestamp = AV_NOPTS_VALUE;
if ((ret = save_avio_options(s)) < 0)
goto fail;
av_dict_set_int(&c->avio_opts, "seekable", c->http_seekable, 0);
if ((ret = parse_playlist(c, s->url, NULL, s->pb)) < 0)
goto fail;
if (c->n_variants == 0) {
av_log(s, AV_LOG_WARNING, "Empty playlist\n");
ret = AVERROR_EOF;
goto fail;
}
if (c->n_playlists > 1 || c->playlists[0]->n_segments == 0) {
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
pls->m3u8_hold_counters = 0;
if ((ret = parse_playlist(c, pls->url, pls, NULL)) < 0) {
av_log(s, AV_LOG_WARNING, "parse_playlist error %s [%s]\n", av_err2str(ret), pls->url);
pls->broken = 1;
if (c->n_playlists > 1)
continue;
goto fail;
}
}
}
for (i = 0; i < c->n_variants; i++) {
if (c->variants[i]->playlists[0]->n_segments == 0) {
av_log(s, AV_LOG_WARNING, "Empty segment [%s]\n", c->variants[i]->playlists[0]->url);
c->variants[i]->playlists[0]->broken = 1;
}
}
if (c->variants[0]->playlists[0]->finished) {
int64_t duration = 0;
for (i = 0; i < c->variants[0]->playlists[0]->n_segments; i++)
duration += c->variants[0]->playlists[0]->segments[i]->duration;
s->duration = duration;
}
for (i = 0; i < c->n_variants; i++) {
struct variant *var = c->variants[i];
if (var->audio_group[0])
add_renditions_to_variant(c, var, AVMEDIA_TYPE_AUDIO, var->audio_group);
if (var->video_group[0])
add_renditions_to_variant(c, var, AVMEDIA_TYPE_VIDEO, var->video_group);
if (var->subtitles_group[0])
add_renditions_to_variant(c, var, AVMEDIA_TYPE_SUBTITLE, var->subtitles_group);
}
for (i = 0; i < c->n_variants; i++) {
struct variant *v = c->variants[i];
AVProgram *program;
program = av_new_program(s, i);
if (!program)
goto fail;
av_dict_set_int(&program->metadata, "variant_bitrate", v->bandwidth, 0);
}
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
if (pls->n_segments == 0)
continue;
pls->cur_seq_no = select_cur_seq_no(c, pls);
highest_cur_seq_no = FFMAX(highest_cur_seq_no, pls->cur_seq_no);
}
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
ff_const59 AVInputFormat *in_fmt = NULL;
if (!(pls->ctx = avformat_alloc_context())) {
ret = AVERROR(ENOMEM);
goto fail;
}
if (pls->n_segments == 0)
continue;
pls->index = i;
pls->needed = 1;
pls->parent = s;
if (!pls->finished && pls->cur_seq_no == highest_cur_seq_no - 1 &&
highest_cur_seq_no < pls->start_seq_no + pls->n_segments) {
pls->cur_seq_no = highest_cur_seq_no;
}
pls->read_buffer = av_malloc(INITIAL_BUFFER_SIZE);
if (!pls->read_buffer){
ret = AVERROR(ENOMEM);
avformat_free_context(pls->ctx);
pls->ctx = NULL;
goto fail;
}
ffio_init_context(&pls->pb, pls->read_buffer, INITIAL_BUFFER_SIZE, 0, pls,
read_data, NULL, NULL);
pls->ctx->probesize = s->probesize > 0 ? s->probesize : 1024 * 4;
pls->ctx->max_analyze_duration = s->max_analyze_duration > 0 ? s->max_analyze_duration : 4 * AV_TIME_BASE;
ret = av_probe_input_buffer(&pls->pb, &in_fmt, pls->segments[0]->url,
NULL, 0, 0);
if (ret < 0) {
av_log(s, AV_LOG_ERROR, "Error when loading first segment '%s'\n", pls->segments[0]->url);
avformat_free_context(pls->ctx);
pls->ctx = NULL;
goto fail;
}
pls->ctx->pb = &pls->pb;
pls->ctx->io_open = nested_io_open;
pls->ctx->flags |= s->flags & ~AVFMT_FLAG_CUSTOM_IO;
if ((ret = ff_copy_whiteblacklists(pls->ctx, s)) < 0)
goto fail;
ret = avformat_open_input(&pls->ctx, pls->segments[0]->url, in_fmt, NULL);
if (ret < 0)
goto fail;
if (pls->id3_deferred_extra && pls->ctx->nb_streams == 1) {
ff_id3v2_parse_apic(pls->ctx, &pls->id3_deferred_extra);
avformat_queue_attached_pictures(pls->ctx);
ff_id3v2_parse_priv(pls->ctx, &pls->id3_deferred_extra);
ff_id3v2_free_extra_meta(&pls->id3_deferred_extra);
pls->id3_deferred_extra = NULL;
}
if (pls->is_id3_timestamped == -1)
av_log(s, AV_LOG_WARNING, "No expected HTTP requests have been made\n");
if (pls->is_id3_timestamped || (pls->n_renditions > 0 && pls->renditions[0]->type == AVMEDIA_TYPE_AUDIO)) {
ret = avformat_find_stream_info(pls->ctx, NULL);
if (ret < 0)
goto fail;
}
pls->has_noheader_flag = !!(pls->ctx->ctx_flags & AVFMTCTX_NOHEADER);
ret = update_streams_from_subdemuxer(s, pls);
if (ret < 0)
goto fail;
if (pls->n_main_streams)
av_dict_copy(&pls->main_streams[0]->metadata, pls->ctx->metadata, 0);
add_metadata_from_renditions(s, pls, AVMEDIA_TYPE_AUDIO);
add_metadata_from_renditions(s, pls, AVMEDIA_TYPE_VIDEO);
add_metadata_from_renditions(s, pls, AVMEDIA_TYPE_SUBTITLE);
}
update_noheader_flag(s);
return 0;
fail:
hls_close(s);
return ret;
}
static int recheck_discard_flags(AVFormatContext *s, int first)
{
HLSContext *c = s->priv_data;
int i, changed = 0;
int cur_needed;
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
cur_needed = playlist_needed(c->playlists[i]);
if (pls->broken) {
continue;
}
if (cur_needed && !pls->needed) {
pls->needed = 1;
changed = 1;
pls->cur_seq_no = select_cur_seq_no(c, pls);
pls->pb.eof_reached = 0;
if (c->cur_timestamp != AV_NOPTS_VALUE) {
pls->seek_timestamp = c->cur_timestamp;
pls->seek_flags = AVSEEK_FLAG_ANY;
pls->seek_stream_index = -1;
}
av_log(s, AV_LOG_INFO, "Now receiving playlist %d, segment %d\n", i, pls->cur_seq_no);
} else if (first && !cur_needed && pls->needed) {
ff_format_io_close(pls->parent, &pls->input);
pls->input_read_done = 0;
ff_format_io_close(pls->parent, &pls->input_next);
pls->input_next_requested = 0;
pls->needed = 0;
changed = 1;
av_log(s, AV_LOG_INFO, "No longer receiving playlist %d\n", i);
}
}
return changed;
}
static void fill_timing_for_id3_timestamped_stream(struct playlist *pls)
{
if (pls->id3_offset >= 0) {
pls->pkt.dts = pls->id3_mpegts_timestamp +
av_rescale_q(pls->id3_offset,
pls->ctx->streams[pls->pkt.stream_index]->time_base,
MPEG_TIME_BASE_Q);
if (pls->pkt.duration)
pls->id3_offset += pls->pkt.duration;
else
pls->id3_offset = -1;
} else {
pls->pkt.dts = AV_NOPTS_VALUE;
}
if (pls->pkt.duration)
pls->pkt.duration = av_rescale_q(pls->pkt.duration,
pls->ctx->streams[pls->pkt.stream_index]->time_base,
MPEG_TIME_BASE_Q);
pls->pkt.pts = AV_NOPTS_VALUE;
}
static AVRational get_timebase(struct playlist *pls)
{
if (pls->is_id3_timestamped)
return MPEG_TIME_BASE_Q;
return pls->ctx->streams[pls->pkt.stream_index]->time_base;
}
static int compare_ts_with_wrapdetect(int64_t ts_a, struct playlist *pls_a,
int64_t ts_b, struct playlist *pls_b)
{
int64_t scaled_ts_a = av_rescale_q(ts_a, get_timebase(pls_a), MPEG_TIME_BASE_Q);
int64_t scaled_ts_b = av_rescale_q(ts_b, get_timebase(pls_b), MPEG_TIME_BASE_Q);
return av_compare_mod(scaled_ts_a, scaled_ts_b, 1LL << 33);
}
static int hls_read_packet(AVFormatContext *s, AVPacket *pkt)
{
HLSContext *c = s->priv_data;
int ret, i, minplaylist = -1;
recheck_discard_flags(s, c->first_packet);
c->first_packet = 0;
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
if (pls->needed && !pls->pkt.data) {
while (1) {
int64_t ts_diff;
AVRational tb;
ret = av_read_frame(pls->ctx, &pls->pkt);
if (ret < 0) {
if (!avio_feof(&pls->pb) && ret != AVERROR_EOF)
return ret;
reset_packet(&pls->pkt);
break;
} else {
if (pls->is_id3_timestamped && pls->pkt.stream_index == 0) {
fill_timing_for_id3_timestamped_stream(pls);
}
if (c->first_timestamp == AV_NOPTS_VALUE &&
pls->pkt.dts != AV_NOPTS_VALUE)
c->first_timestamp = av_rescale_q(pls->pkt.dts,
get_timebase(pls), AV_TIME_BASE_Q);
}
if (pls->seek_timestamp == AV_NOPTS_VALUE)
break;
if (pls->seek_stream_index < 0 ||
pls->seek_stream_index == pls->pkt.stream_index) {
if (pls->pkt.dts == AV_NOPTS_VALUE) {
pls->seek_timestamp = AV_NOPTS_VALUE;
break;
}
tb = get_timebase(pls);
ts_diff = av_rescale_rnd(pls->pkt.dts, AV_TIME_BASE,
tb.den, AV_ROUND_DOWN) -
pls->seek_timestamp;
if (ts_diff >= 0 && (pls->seek_flags & AVSEEK_FLAG_ANY ||
pls->pkt.flags & AV_PKT_FLAG_KEY)) {
pls->seek_timestamp = AV_NOPTS_VALUE;
break;
}
}
av_packet_unref(&pls->pkt);
}
}
if (pls->pkt.data) {
struct playlist *minpls = minplaylist < 0 ?
NULL : c->playlists[minplaylist];
if (minplaylist < 0) {
minplaylist = i;
} else {
int64_t dts = pls->pkt.dts;
int64_t mindts = minpls->pkt.dts;
if (dts == AV_NOPTS_VALUE ||
(mindts != AV_NOPTS_VALUE && compare_ts_with_wrapdetect(dts, pls, mindts, minpls) < 0))
minplaylist = i;
}
}
}
if (minplaylist >= 0) {
struct playlist *pls = c->playlists[minplaylist];
AVStream *ist;
AVStream *st;
ret = update_streams_from_subdemuxer(s, pls);
if (ret < 0) {
av_packet_unref(&pls->pkt);
return ret;
}
if (pls->ctx->event_flags & AVFMT_EVENT_FLAG_METADATA_UPDATED) {
if (pls->n_main_streams) {
st = pls->main_streams[0];
av_dict_copy(&st->metadata, pls->ctx->metadata, 0);
st->event_flags |= AVSTREAM_EVENT_FLAG_METADATA_UPDATED;
}
pls->ctx->event_flags &= ~AVFMT_EVENT_FLAG_METADATA_UPDATED;
}
if (pls->has_noheader_flag && !(pls->ctx->ctx_flags & AVFMTCTX_NOHEADER)) {
pls->has_noheader_flag = 0;
update_noheader_flag(s);
}
if (pls->pkt.stream_index >= pls->n_main_streams) {
av_log(s, AV_LOG_ERROR, "stream index inconsistency: index %d, %d main streams, %d subdemuxer streams\n",
pls->pkt.stream_index, pls->n_main_streams, pls->ctx->nb_streams);
av_packet_unref(&pls->pkt);
return AVERROR_BUG;
}
ist = pls->ctx->streams[pls->pkt.stream_index];
st = pls->main_streams[pls->pkt.stream_index];
av_packet_move_ref(pkt, &pls->pkt);
pkt->stream_index = st->index;
if (pkt->dts != AV_NOPTS_VALUE)
c->cur_timestamp = av_rescale_q(pkt->dts,
ist->time_base,
AV_TIME_BASE_Q);
if (ist->codecpar->codec_id != st->codecpar->codec_id) {
ret = set_stream_info_from_input_stream(st, pls, ist);
if (ret < 0) {
return ret;
}
}
return 0;
}
return AVERROR_EOF;
}
static int hls_read_seek(AVFormatContext *s, int stream_index,
int64_t timestamp, int flags)
{
HLSContext *c = s->priv_data;
struct playlist *seek_pls = NULL;
int i, seq_no;
int j;
int stream_subdemuxer_index;
int64_t first_timestamp, seek_timestamp, duration;
if ((flags & AVSEEK_FLAG_BYTE) || (c->ctx->ctx_flags & AVFMTCTX_UNSEEKABLE))
return AVERROR(ENOSYS);
first_timestamp = c->first_timestamp == AV_NOPTS_VALUE ?
0 : c->first_timestamp;
seek_timestamp = av_rescale_rnd(timestamp, AV_TIME_BASE,
s->streams[stream_index]->time_base.den,
flags & AVSEEK_FLAG_BACKWARD ?
AV_ROUND_DOWN : AV_ROUND_UP);
duration = s->duration == AV_NOPTS_VALUE ?
0 : s->duration;
if (0 < duration && duration < seek_timestamp - first_timestamp)
return AVERROR(EIO);
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
for (j = 0; j < pls->n_main_streams; j++) {
if (pls->main_streams[j] == s->streams[stream_index]) {
seek_pls = pls;
stream_subdemuxer_index = j;
break;
}
}
}
if (!seek_pls || !find_timestamp_in_playlist(c, seek_pls, seek_timestamp, &seq_no))
return AVERROR(EIO);
seek_pls->cur_seq_no = seq_no;
seek_pls->seek_stream_index = stream_subdemuxer_index;
for (i = 0; i < c->n_playlists; i++) {
struct playlist *pls = c->playlists[i];
ff_format_io_close(pls->parent, &pls->input);
pls->input_read_done = 0;
ff_format_io_close(pls->parent, &pls->input_next);
pls->input_next_requested = 0;
av_packet_unref(&pls->pkt);
pls->pb.eof_reached = 0;
pls->pb.buf_end = pls->pb.buf_ptr = pls->pb.buffer;
pls->pb.pos = 0;
ff_read_frame_flush(pls->ctx);
pls->seek_timestamp = seek_timestamp;
pls->seek_flags = flags;
if (pls != seek_pls) {
find_timestamp_in_playlist(c, pls, seek_timestamp, &pls->cur_seq_no);
pls->seek_stream_index = -1;
pls->seek_flags |= AVSEEK_FLAG_ANY;
}
}
c->cur_timestamp = seek_timestamp;
return 0;
}
static int hls_probe(const AVProbeData *p)
{
if (strncmp(p->buf, "#EXTM3U", 7))
return 0;
if (strstr(p->buf, "#EXT-X-STREAM-INF:") ||
strstr(p->buf, "#EXT-X-TARGETDURATION:") ||
strstr(p->buf, "#EXT-X-MEDIA-SEQUENCE:"))
return AVPROBE_SCORE_MAX;
return 0;
}
#define OFFSET(x) offsetof(HLSContext, x)
#define FLAGS AV_OPT_FLAG_DECODING_PARAM
static const AVOption hls_options[] = {
{"live_start_index", "segment index to start live streams at (negative values are from the end)",
OFFSET(live_start_index), AV_OPT_TYPE_INT, {.i64 = -3}, INT_MIN, INT_MAX, FLAGS},
{"allowed_extensions", "List of file extensions that hls is allowed to access",
OFFSET(allowed_extensions), AV_OPT_TYPE_STRING,
{.str = "3gp,aac,avi,flac,mkv,m3u8,m4a,m4s,m4v,mpg,mov,mp2,mp3,mp4,mpeg,mpegts,ogg,ogv,oga,ts,vob,wav"},
INT_MIN, INT_MAX, FLAGS},
{"max_reload", "Maximum number of times a insufficient list is attempted to be reloaded",
OFFSET(max_reload), AV_OPT_TYPE_INT, {.i64 = 1000}, 0, INT_MAX, FLAGS},
{"m3u8_hold_counters", "The maximum number of times to load m3u8 when it refreshes without new segments",
OFFSET(m3u8_hold_counters), AV_OPT_TYPE_INT, {.i64 = 1000}, 0, INT_MAX, FLAGS},
{"http_persistent", "Use persistent HTTP connections",
OFFSET(http_persistent), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, FLAGS },
{"http_multiple", "Use multiple HTTP connections for fetching segments",
OFFSET(http_multiple), AV_OPT_TYPE_BOOL, {.i64 = -1}, -1, 1, FLAGS},
{"http_seekable", "Use HTTP partial requests, 0 = disable, 1 = enable, -1 = auto",
OFFSET(http_seekable), AV_OPT_TYPE_BOOL, { .i64 = -1}, -1, 1, FLAGS},
{NULL}
};
static const AVClass hls_class = {
.class_name = "hls demuxer",
.item_name = av_default_item_name,
.option = hls_options,
.version = LIBAVUTIL_VERSION_INT,
};
AVInputFormat ff_hls_demuxer = {
.name = "hls",
.long_name = NULL_IF_CONFIG_SMALL("Apple HTTP Live Streaming"),
.priv_class = &hls_class,
.priv_data_size = sizeof(HLSContext),
.flags = AVFMT_NOGENSEARCH | AVFMT_TS_DISCONT,
.read_probe = hls_probe,
.read_header = hls_read_header,
.read_packet = hls_read_packet,
.read_close = hls_close,
.read_seek = hls_read_seek,
};
