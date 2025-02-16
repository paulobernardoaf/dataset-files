
















#pragma once

#include "../util/c99defs.h"
#include "audio-io.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct audio_resampler;
typedef struct audio_resampler audio_resampler_t;

struct resample_info {
uint32_t samples_per_sec;
enum audio_format format;
enum speaker_layout speakers;
};

EXPORT audio_resampler_t *
audio_resampler_create(const struct resample_info *dst,
const struct resample_info *src);
EXPORT void audio_resampler_destroy(audio_resampler_t *resampler);

EXPORT bool audio_resampler_resample(audio_resampler_t *resampler,
uint8_t *output[], uint32_t *out_frames,
uint64_t *ts_offset,
const uint8_t *const input[],
uint32_t in_frames);

#if defined(__cplusplus)
}
#endif
