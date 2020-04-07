#pragma once
#include "obs.h"
#if defined(__cplusplus)
extern "C" {
#endif
enum obs_fader_type {
OBS_FADER_CUBIC,
OBS_FADER_IEC,
OBS_FADER_LOG
};
enum obs_peak_meter_type {
SAMPLE_PEAK_METER,
TRUE_PEAK_METER
};
EXPORT obs_fader_t *obs_fader_create(enum obs_fader_type type);
EXPORT void obs_fader_destroy(obs_fader_t *fader);
EXPORT bool obs_fader_set_db(obs_fader_t *fader, const float db);
EXPORT float obs_fader_get_db(obs_fader_t *fader);
EXPORT bool obs_fader_set_deflection(obs_fader_t *fader, const float def);
EXPORT float obs_fader_get_deflection(obs_fader_t *fader);
EXPORT bool obs_fader_set_mul(obs_fader_t *fader, const float mul);
EXPORT float obs_fader_get_mul(obs_fader_t *fader);
EXPORT bool obs_fader_attach_source(obs_fader_t *fader, obs_source_t *source);
EXPORT void obs_fader_detach_source(obs_fader_t *fader);
typedef void (*obs_fader_changed_t)(void *param, float db);
EXPORT void obs_fader_add_callback(obs_fader_t *fader,
obs_fader_changed_t callback, void *param);
EXPORT void obs_fader_remove_callback(obs_fader_t *fader,
obs_fader_changed_t callback,
void *param);
EXPORT obs_volmeter_t *obs_volmeter_create(enum obs_fader_type type);
EXPORT void obs_volmeter_destroy(obs_volmeter_t *volmeter);
EXPORT bool obs_volmeter_attach_source(obs_volmeter_t *volmeter,
obs_source_t *source);
EXPORT void obs_volmeter_detach_source(obs_volmeter_t *volmeter);
EXPORT void
obs_volmeter_set_peak_meter_type(obs_volmeter_t *volmeter,
enum obs_peak_meter_type peak_meter_type);
EXPORT void obs_volmeter_set_update_interval(obs_volmeter_t *volmeter,
const unsigned int ms);
EXPORT unsigned int obs_volmeter_get_update_interval(obs_volmeter_t *volmeter);
EXPORT int obs_volmeter_get_nr_channels(obs_volmeter_t *volmeter);
typedef void (*obs_volmeter_updated_t)(
void *param, const float magnitude[MAX_AUDIO_CHANNELS],
const float peak[MAX_AUDIO_CHANNELS],
const float input_peak[MAX_AUDIO_CHANNELS]);
EXPORT void obs_volmeter_add_callback(obs_volmeter_t *volmeter,
obs_volmeter_updated_t callback,
void *param);
EXPORT void obs_volmeter_remove_callback(obs_volmeter_t *volmeter,
obs_volmeter_updated_t callback,
void *param);
EXPORT float obs_mul_to_db(float mul);
EXPORT float obs_db_to_mul(float db);
#if defined(__cplusplus)
}
#endif
