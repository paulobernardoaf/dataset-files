
















#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <obs.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4204)
#endif

#include <libavcodec/avcodec.h>
#include <libavutil/log.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

struct ffmpeg_decode {
AVCodecContext *decoder;
AVCodec *codec;

AVFrame *hw_frame;
AVFrame *frame;
bool hw;

uint8_t *packet_buffer;
size_t packet_size;
};

extern int ffmpeg_decode_init(struct ffmpeg_decode *decode, enum AVCodecID id,
bool use_hw);
extern void ffmpeg_decode_free(struct ffmpeg_decode *decode);

extern bool ffmpeg_decode_audio(struct ffmpeg_decode *decode, uint8_t *data,
size_t size, struct obs_source_audio *audio,
bool *got_output);

extern bool ffmpeg_decode_video(struct ffmpeg_decode *decode, uint8_t *data,
size_t size, long long *ts,
enum video_range_type range,
struct obs_source_frame2 *frame,
bool *got_output);

static inline bool ffmpeg_decode_valid(struct ffmpeg_decode *decode)
{
return decode->decoder != NULL;
}

#if defined(__cplusplus)
}
#endif
