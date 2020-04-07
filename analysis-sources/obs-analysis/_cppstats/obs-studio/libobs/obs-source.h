#pragma once
#include "obs.h"
#if defined(__cplusplus)
extern "C" {
#endif
enum obs_source_type {
OBS_SOURCE_TYPE_INPUT,
OBS_SOURCE_TYPE_FILTER,
OBS_SOURCE_TYPE_TRANSITION,
OBS_SOURCE_TYPE_SCENE,
};
enum obs_balance_type {
OBS_BALANCE_TYPE_SINE_LAW,
OBS_BALANCE_TYPE_SQUARE_LAW,
OBS_BALANCE_TYPE_LINEAR,
};
enum obs_icon_type {
OBS_ICON_TYPE_UNKNOWN,
OBS_ICON_TYPE_IMAGE,
OBS_ICON_TYPE_COLOR,
OBS_ICON_TYPE_SLIDESHOW,
OBS_ICON_TYPE_AUDIO_INPUT,
OBS_ICON_TYPE_AUDIO_OUTPUT,
OBS_ICON_TYPE_DESKTOP_CAPTURE,
OBS_ICON_TYPE_WINDOW_CAPTURE,
OBS_ICON_TYPE_GAME_CAPTURE,
OBS_ICON_TYPE_CAMERA,
OBS_ICON_TYPE_TEXT,
OBS_ICON_TYPE_MEDIA,
OBS_ICON_TYPE_BROWSER,
OBS_ICON_TYPE_CUSTOM,
};
enum obs_media_state {
OBS_MEDIA_STATE_NONE,
OBS_MEDIA_STATE_PLAYING,
OBS_MEDIA_STATE_OPENING,
OBS_MEDIA_STATE_BUFFERING,
OBS_MEDIA_STATE_PAUSED,
OBS_MEDIA_STATE_STOPPED,
OBS_MEDIA_STATE_ENDED,
OBS_MEDIA_STATE_ERROR,
};
#define OBS_SOURCE_VIDEO (1 << 0)
#define OBS_SOURCE_AUDIO (1 << 1)
#define OBS_SOURCE_ASYNC (1 << 2)
#define OBS_SOURCE_ASYNC_VIDEO (OBS_SOURCE_ASYNC | OBS_SOURCE_VIDEO)
#define OBS_SOURCE_CUSTOM_DRAW (1 << 3)
#define OBS_SOURCE_INTERACTION (1 << 5)
#define OBS_SOURCE_COMPOSITE (1 << 6)
#define OBS_SOURCE_DO_NOT_DUPLICATE (1 << 7)
#define OBS_SOURCE_DEPRECATED (1 << 8)
#define OBS_SOURCE_DO_NOT_SELF_MONITOR (1 << 9)
#define OBS_SOURCE_CAP_DISABLED (1 << 10)
#define OBS_SOURCE_CAP_OBSOLETE OBS_SOURCE_CAP_DISABLED
#define OBS_SOURCE_MONITOR_BY_DEFAULT (1 << 11)
#define OBS_SOURCE_SUBMIX (1 << 12)
#define OBS_SOURCE_CONTROLLABLE_MEDIA (1 << 13)
typedef void (*obs_source_enum_proc_t)(obs_source_t *parent,
obs_source_t *child, void *param);
struct obs_source_audio_mix {
struct audio_output_data output[MAX_AUDIO_MIXES];
};
struct obs_source_info {
const char *id;
enum obs_source_type type;
uint32_t output_flags;
const char *(*get_name)(void *type_data);
void *(*create)(obs_data_t *settings, obs_source_t *source);
void (*destroy)(void *data);
uint32_t (*get_width)(void *data);
uint32_t (*get_height)(void *data);
void (*get_defaults)(obs_data_t *settings);
obs_properties_t *(*get_properties)(void *data);
void (*update)(void *data, obs_data_t *settings);
void (*activate)(void *data);
void (*deactivate)(void *data);
void (*show)(void *data);
void (*hide)(void *data);
void (*video_tick)(void *data, float seconds);
void (*video_render)(void *data, gs_effect_t *effect);
struct obs_source_frame *(*filter_video)(
void *data, struct obs_source_frame *frame);
struct obs_audio_data *(*filter_audio)(void *data,
struct obs_audio_data *audio);
void (*enum_active_sources)(void *data,
obs_source_enum_proc_t enum_callback,
void *param);
void (*save)(void *data, obs_data_t *settings);
void (*load)(void *data, obs_data_t *settings);
void (*mouse_click)(void *data, const struct obs_mouse_event *event,
int32_t type, bool mouse_up, uint32_t click_count);
void (*mouse_move)(void *data, const struct obs_mouse_event *event,
bool mouse_leave);
void (*mouse_wheel)(void *data, const struct obs_mouse_event *event,
int x_delta, int y_delta);
void (*focus)(void *data, bool focus);
void (*key_click)(void *data, const struct obs_key_event *event,
bool key_up);
void (*filter_remove)(void *data, obs_source_t *source);
void *type_data;
void (*free_type_data)(void *type_data);
bool (*audio_render)(void *data, uint64_t *ts_out,
struct obs_source_audio_mix *audio_output,
uint32_t mixers, size_t channels,
size_t sample_rate);
void (*enum_all_sources)(void *data,
obs_source_enum_proc_t enum_callback,
void *param);
void (*transition_start)(void *data);
void (*transition_stop)(void *data);
void (*get_defaults2)(void *type_data, obs_data_t *settings);
obs_properties_t *(*get_properties2)(void *data, void *type_data);
bool (*audio_mix)(void *data, uint64_t *ts_out,
struct audio_output_data *audio_output,
size_t channels, size_t sample_rate);
enum obs_icon_type icon_type;
void (*media_play_pause)(void *data, bool pause);
void (*media_restart)(void *data);
void (*media_stop)(void *data);
void (*media_next)(void *data);
void (*media_previous)(void *data);
int64_t (*media_get_duration)(void *data);
int64_t (*media_get_time)(void *data);
void (*media_set_time)(void *data, int64_t miliseconds);
enum obs_media_state (*media_get_state)(void *data);
uint32_t version; 
const char *unversioned_id; 
};
EXPORT void obs_register_source_s(const struct obs_source_info *info,
size_t size);
#define obs_register_source(info) obs_register_source_s(info, sizeof(struct obs_source_info))
#if defined(__cplusplus)
}
#endif
