#include <stdint.h>
#include "libavutil/common.h"
#include "avformat.h"
#include "avio.h"
typedef enum {
PLAYLIST_TYPE_NONE,
PLAYLIST_TYPE_EVENT,
PLAYLIST_TYPE_VOD,
PLAYLIST_TYPE_NB,
} PlaylistType;
void ff_hls_write_playlist_version(AVIOContext *out, int version);
void ff_hls_write_audio_rendition(AVIOContext *out, char *agroup,
const char *filename, char *language, int name_id, int is_default);
void ff_hls_write_stream_info(AVStream *st, AVIOContext *out,
int bandwidth, const char *filename, char *agroup,
char *codecs, char *ccgroup);
void ff_hls_write_playlist_header(AVIOContext *out, int version, int allowcache,
int target_duration, int64_t sequence,
uint32_t playlist_type, int iframe_mode);
void ff_hls_write_init_file(AVIOContext *out, char *filename,
int byterange_mode, int64_t size, int64_t pos);
int ff_hls_write_file_entry(AVIOContext *out, int insert_discont,
int byterange_mode,
double duration, int round_duration,
int64_t size, int64_t pos, 
char *baseurl, 
char *filename, double *prog_date_time,
int64_t video_keyframe_size, int64_t video_keyframe_pos, int iframe_mode);
void ff_hls_write_end_list (AVIOContext *out);
