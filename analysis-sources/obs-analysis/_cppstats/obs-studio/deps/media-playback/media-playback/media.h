#pragma once
#include <obs.h>
#include "decode.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4204)
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <util/threading.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
typedef void (*mp_video_cb)(void *opaque, struct obs_source_frame *frame);
typedef void (*mp_audio_cb)(void *opaque, struct obs_source_audio *audio);
typedef void (*mp_stop_cb)(void *opaque);
struct mp_media {
AVFormatContext *fmt;
mp_video_cb v_preload_cb;
mp_stop_cb stop_cb;
mp_video_cb v_cb;
mp_audio_cb a_cb;
void *opaque;
char *path;
char *format_name;
int buffering;
int speed;
enum AVPixelFormat scale_format;
struct SwsContext *swscale;
int scale_linesizes[4];
uint8_t *scale_pic[4];
struct mp_decode v;
struct mp_decode a;
bool is_local_file;
bool has_video;
bool has_audio;
bool is_file;
bool eof;
bool hw;
struct obs_source_frame obsframe;
enum video_colorspace cur_space;
enum video_range_type cur_range;
enum video_range_type force_range;
int64_t play_sys_ts;
int64_t next_pts_ns;
uint64_t next_ns;
int64_t start_ts;
int64_t base_ts;
uint64_t interrupt_poll_ts;
pthread_mutex_t mutex;
os_sem_t *sem;
bool stopping;
bool looping;
bool active;
bool reset;
bool kill;
bool thread_valid;
pthread_t thread;
bool pause;
bool reset_ts;
bool seek;
int64_t seek_pos;
};
typedef struct mp_media mp_media_t;
struct mp_media_info {
void *opaque;
mp_video_cb v_cb;
mp_video_cb v_preload_cb;
mp_audio_cb a_cb;
mp_stop_cb stop_cb;
const char *path;
const char *format;
int buffering;
int speed;
enum video_range_type force_range;
bool hardware_decoding;
bool is_local_file;
};
extern bool mp_media_init(mp_media_t *media, const struct mp_media_info *info);
extern void mp_media_free(mp_media_t *media);
extern void mp_media_play(mp_media_t *media, bool loop);
extern void mp_media_stop(mp_media_t *media);
extern void mp_media_play_pause(mp_media_t *media, bool pause);
extern int64_t mp_get_current_time(mp_media_t *m);
extern void mp_media_seek_to(mp_media_t *m, int64_t pos);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
#define USE_NEW_FFMPEG_DECODE_API
#endif
#if defined(__cplusplus)
}
#endif
