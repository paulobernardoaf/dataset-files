
















#pragma once

#include "media-io-defs.h"
#include "../util/c99defs.h"
#include "../util/util_uint128.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_AUDIO_MIXES 6
#define MAX_AUDIO_CHANNELS 8
#define AUDIO_OUTPUT_FRAMES 1024

#define TOTAL_AUDIO_SIZE (MAX_AUDIO_MIXES * MAX_AUDIO_CHANNELS * AUDIO_OUTPUT_FRAMES * sizeof(float))








struct audio_output;
typedef struct audio_output audio_t;

enum audio_format {
AUDIO_FORMAT_UNKNOWN,

AUDIO_FORMAT_U8BIT,
AUDIO_FORMAT_16BIT,
AUDIO_FORMAT_32BIT,
AUDIO_FORMAT_FLOAT,

AUDIO_FORMAT_U8BIT_PLANAR,
AUDIO_FORMAT_16BIT_PLANAR,
AUDIO_FORMAT_32BIT_PLANAR,
AUDIO_FORMAT_FLOAT_PLANAR,
};










enum speaker_layout {
SPEAKERS_UNKNOWN, 
SPEAKERS_MONO, 
SPEAKERS_STEREO, 
SPEAKERS_2POINT1, 
SPEAKERS_4POINT0, 
SPEAKERS_4POINT1, 
SPEAKERS_5POINT1, 
SPEAKERS_7POINT1 = 8, 
};

struct audio_data {
uint8_t *data[MAX_AV_PLANES];
uint32_t frames;
uint64_t timestamp;
};

struct audio_output_data {
float *data[MAX_AUDIO_CHANNELS];
};

typedef bool (*audio_input_callback_t)(void *param, uint64_t start_ts,
uint64_t end_ts, uint64_t *new_ts,
uint32_t active_mixers,
struct audio_output_data *mixes);

struct audio_output_info {
const char *name;

uint32_t samples_per_sec;
enum audio_format format;
enum speaker_layout speakers;

audio_input_callback_t input_callback;
void *input_param;
};

struct audio_convert_info {
uint32_t samples_per_sec;
enum audio_format format;
enum speaker_layout speakers;
};

static inline uint32_t get_audio_channels(enum speaker_layout speakers)
{
switch (speakers) {
case SPEAKERS_MONO:
return 1;
case SPEAKERS_STEREO:
return 2;
case SPEAKERS_2POINT1:
return 3;
case SPEAKERS_4POINT0:
return 4;
case SPEAKERS_4POINT1:
return 5;
case SPEAKERS_5POINT1:
return 6;
case SPEAKERS_7POINT1:
return 8;
case SPEAKERS_UNKNOWN:
return 0;
}

return 0;
}

static inline size_t get_audio_bytes_per_channel(enum audio_format format)
{
switch (format) {
case AUDIO_FORMAT_U8BIT:
case AUDIO_FORMAT_U8BIT_PLANAR:
return 1;

case AUDIO_FORMAT_16BIT:
case AUDIO_FORMAT_16BIT_PLANAR:
return 2;

case AUDIO_FORMAT_FLOAT:
case AUDIO_FORMAT_FLOAT_PLANAR:
case AUDIO_FORMAT_32BIT:
case AUDIO_FORMAT_32BIT_PLANAR:
return 4;

case AUDIO_FORMAT_UNKNOWN:
return 0;
}

return 0;
}

static inline bool is_audio_planar(enum audio_format format)
{
switch (format) {
case AUDIO_FORMAT_U8BIT:
case AUDIO_FORMAT_16BIT:
case AUDIO_FORMAT_32BIT:
case AUDIO_FORMAT_FLOAT:
return false;

case AUDIO_FORMAT_U8BIT_PLANAR:
case AUDIO_FORMAT_FLOAT_PLANAR:
case AUDIO_FORMAT_16BIT_PLANAR:
case AUDIO_FORMAT_32BIT_PLANAR:
return true;

case AUDIO_FORMAT_UNKNOWN:
return false;
}

return false;
}

static inline size_t get_audio_planes(enum audio_format format,
enum speaker_layout speakers)
{
return (is_audio_planar(format) ? get_audio_channels(speakers) : 1);
}

static inline size_t get_audio_size(enum audio_format format,
enum speaker_layout speakers,
uint32_t frames)
{
bool planar = is_audio_planar(format);

return (planar ? 1 : get_audio_channels(speakers)) *
get_audio_bytes_per_channel(format) * frames;
}

static inline uint64_t audio_frames_to_ns(size_t sample_rate, uint64_t frames)
{
util_uint128_t val;
val = util_mul64_64(frames, 1000000000ULL);
val = util_div128_32(val, (uint32_t)sample_rate);
return val.low;
}

static inline uint64_t ns_to_audio_frames(size_t sample_rate, uint64_t frames)
{
util_uint128_t val;
val = util_mul64_64(frames, sample_rate);
val = util_div128_32(val, 1000000000);
return val.low;
}

#define AUDIO_OUTPUT_SUCCESS 0
#define AUDIO_OUTPUT_INVALIDPARAM -1
#define AUDIO_OUTPUT_FAIL -2

EXPORT int audio_output_open(audio_t **audio, struct audio_output_info *info);
EXPORT void audio_output_close(audio_t *audio);

typedef void (*audio_output_callback_t)(void *param, size_t mix_idx,
struct audio_data *data);

EXPORT bool audio_output_connect(audio_t *video, size_t mix_idx,
const struct audio_convert_info *conversion,
audio_output_callback_t callback, void *param);
EXPORT void audio_output_disconnect(audio_t *video, size_t mix_idx,
audio_output_callback_t callback,
void *param);

EXPORT bool audio_output_active(const audio_t *audio);

EXPORT size_t audio_output_get_block_size(const audio_t *audio);
EXPORT size_t audio_output_get_planes(const audio_t *audio);
EXPORT size_t audio_output_get_channels(const audio_t *audio);
EXPORT uint32_t audio_output_get_sample_rate(const audio_t *audio);
EXPORT const struct audio_output_info *
audio_output_get_info(const audio_t *audio);

#if defined(__cplusplus)
}
#endif
