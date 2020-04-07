
















#pragma once

#include "util/c99defs.h"
#include "util/bmem.h"
#include "util/profiler.h"
#include "util/text-lookup.h"
#include "graphics/graphics.h"
#include "graphics/vec2.h"
#include "graphics/vec3.h"
#include "media-io/audio-io.h"
#include "media-io/video-io.h"
#include "callback/signal.h"
#include "callback/proc.h"

#include "obs-config.h"
#include "obs-defs.h"
#include "obs-data.h"
#include "obs-ui.h"
#include "obs-properties.h"
#include "obs-interaction.h"

struct matrix4;


struct obs_display;
struct obs_view;
struct obs_source;
struct obs_scene;
struct obs_scene_item;
struct obs_output;
struct obs_encoder;
struct obs_service;
struct obs_module;
struct obs_fader;
struct obs_volmeter;

typedef struct obs_display obs_display_t;
typedef struct obs_view obs_view_t;
typedef struct obs_source obs_source_t;
typedef struct obs_scene obs_scene_t;
typedef struct obs_scene_item obs_sceneitem_t;
typedef struct obs_output obs_output_t;
typedef struct obs_encoder obs_encoder_t;
typedef struct obs_service obs_service_t;
typedef struct obs_module obs_module_t;
typedef struct obs_fader obs_fader_t;
typedef struct obs_volmeter obs_volmeter_t;

typedef struct obs_weak_source obs_weak_source_t;
typedef struct obs_weak_output obs_weak_output_t;
typedef struct obs_weak_encoder obs_weak_encoder_t;
typedef struct obs_weak_service obs_weak_service_t;

#include "obs-source.h"
#include "obs-encoder.h"
#include "obs-output.h"
#include "obs-service.h"
#include "obs-audio-controls.h"
#include "obs-hotkey.h"














#if defined(__cplusplus)
extern "C" {
#endif



enum obs_order_movement {
OBS_ORDER_MOVE_UP,
OBS_ORDER_MOVE_DOWN,
OBS_ORDER_MOVE_TOP,
OBS_ORDER_MOVE_BOTTOM,
};






enum obs_allow_direct_render {
OBS_NO_DIRECT_RENDERING,
OBS_ALLOW_DIRECT_RENDERING,
};

enum obs_scale_type {
OBS_SCALE_DISABLE,
OBS_SCALE_POINT,
OBS_SCALE_BICUBIC,
OBS_SCALE_BILINEAR,
OBS_SCALE_LANCZOS,
OBS_SCALE_AREA,
};






enum obs_bounds_type {
OBS_BOUNDS_NONE, 
OBS_BOUNDS_STRETCH, 
OBS_BOUNDS_SCALE_INNER, 
OBS_BOUNDS_SCALE_OUTER, 
OBS_BOUNDS_SCALE_TO_WIDTH, 
OBS_BOUNDS_SCALE_TO_HEIGHT, 
OBS_BOUNDS_MAX_ONLY, 
};

struct obs_transform_info {
struct vec2 pos;
float rot;
struct vec2 scale;
uint32_t alignment;

enum obs_bounds_type bounds_type;
uint32_t bounds_alignment;
struct vec2 bounds;
};




struct obs_video_info {
#if !defined(SWIG)



const char *graphics_module;
#endif

uint32_t fps_num; 
uint32_t fps_den; 

uint32_t base_width; 
uint32_t base_height; 

uint32_t output_width; 
uint32_t output_height; 
enum video_format output_format; 


uint32_t adapter;


bool gpu_conversion;

enum video_colorspace colorspace; 
enum video_range_type range; 

enum obs_scale_type scale_type; 
};




struct obs_audio_info {
uint32_t samples_per_sec;
enum speaker_layout speakers;
};





struct obs_audio_data {
uint8_t *data[MAX_AV_PLANES];
uint32_t frames;
uint64_t timestamp;
};





struct obs_source_audio {
const uint8_t *data[MAX_AV_PLANES];
uint32_t frames;

enum speaker_layout speakers;
enum audio_format format;
uint32_t samples_per_sec;

uint64_t timestamp;
};














struct obs_source_frame {
uint8_t *data[MAX_AV_PLANES];
uint32_t linesize[MAX_AV_PLANES];
uint32_t width;
uint32_t height;
uint64_t timestamp;

enum video_format format;
float color_matrix[16];
bool full_range;
float color_range_min[3];
float color_range_max[3];
bool flip;


volatile long refs;
bool prev_frame;
};

struct obs_source_frame2 {
uint8_t *data[MAX_AV_PLANES];
uint32_t linesize[MAX_AV_PLANES];
uint32_t width;
uint32_t height;
uint64_t timestamp;

enum video_format format;
enum video_range_type range;
float color_matrix[16];
float color_range_min[3];
float color_range_max[3];
bool flip;
};


struct obs_cmdline_args {
int argc;
char **argv;
};










EXPORT char *obs_find_data_file(const char *file);






EXPORT void obs_add_data_path(const char *path);









EXPORT bool obs_remove_data_path(const char *path);









EXPORT bool obs_startup(const char *locale, const char *module_config_path,
profiler_name_store_t *store);


EXPORT void obs_shutdown(void);


EXPORT bool obs_initialized(void);


EXPORT uint32_t obs_get_version(void);


EXPORT const char *obs_get_version_string(void);









EXPORT void obs_set_cmdline_args(int argc, const char *const *argv);







EXPORT struct obs_cmdline_args obs_get_cmdline_args(void);







EXPORT void obs_set_locale(const char *locale);


EXPORT const char *obs_get_locale(void);



#if defined(_WIN32)
EXPORT void obs_init_win32_crash_handler(void);
#endif






EXPORT profiler_name_store_t *obs_get_profiler_name_store(void);

















EXPORT int obs_reset_video(struct obs_video_info *ovi);






EXPORT bool obs_reset_audio(const struct obs_audio_info *oai);


EXPORT bool obs_get_video_info(struct obs_video_info *ovi);


EXPORT bool obs_get_audio_info(struct obs_audio_info *oai);






















EXPORT int obs_open_module(obs_module_t **module, const char *path,
const char *data_path);






EXPORT bool obs_init_module(obs_module_t *module);


EXPORT void obs_log_loaded_modules(void);


EXPORT const char *obs_get_module_file_name(obs_module_t *module);


EXPORT const char *obs_get_module_name(obs_module_t *module);


EXPORT const char *obs_get_module_author(obs_module_t *module);


EXPORT const char *obs_get_module_description(obs_module_t *module);


EXPORT const char *obs_get_module_binary_path(obs_module_t *module);


EXPORT const char *obs_get_module_data_path(obs_module_t *module);









EXPORT void obs_add_module_path(const char *bin, const char *data);


EXPORT void obs_load_all_modules(void);



EXPORT void obs_post_load_modules(void);

#if !defined(SWIG)
struct obs_module_info {
const char *bin_path;
const char *data_path;
};

typedef void (*obs_find_module_callback_t)(void *param,
const struct obs_module_info *info);


EXPORT void obs_find_modules(obs_find_module_callback_t callback, void *param);
#endif

typedef void (*obs_enum_module_callback_t)(void *param, obs_module_t *module);


EXPORT void obs_enum_modules(obs_enum_module_callback_t callback, void *param);


EXPORT lookup_t *obs_module_load_locale(obs_module_t *module,
const char *default_locale,
const char *locale);












EXPORT char *obs_find_module_file(obs_module_t *module, const char *file);












EXPORT char *obs_module_get_config_path(obs_module_t *module, const char *file);


EXPORT bool obs_enum_source_types(size_t idx, const char **id);







EXPORT bool obs_enum_input_types(size_t idx, const char **id);
EXPORT bool obs_enum_input_types2(size_t idx, const char **id,
const char **unversioned_id);

EXPORT const char *obs_get_latest_input_type_id(const char *unversioned_id);







EXPORT bool obs_enum_filter_types(size_t idx, const char **id);







EXPORT bool obs_enum_transition_types(size_t idx, const char **id);


EXPORT bool obs_enum_output_types(size_t idx, const char **id);


EXPORT bool obs_enum_encoder_types(size_t idx, const char **id);


EXPORT bool obs_enum_service_types(size_t idx, const char **id);


EXPORT void obs_enter_graphics(void);


EXPORT void obs_leave_graphics(void);


EXPORT audio_t *obs_get_audio(void);


EXPORT video_t *obs_get_video(void);


EXPORT bool obs_video_active(void);


EXPORT void obs_set_output_source(uint32_t channel, obs_source_t *source);





EXPORT obs_source_t *obs_get_output_source(uint32_t channel);










EXPORT void obs_enum_sources(bool (*enum_proc)(void *, obs_source_t *),
void *param);


EXPORT void obs_enum_scenes(bool (*enum_proc)(void *, obs_source_t *),
void *param);


EXPORT void obs_enum_outputs(bool (*enum_proc)(void *, obs_output_t *),
void *param);


EXPORT void obs_enum_encoders(bool (*enum_proc)(void *, obs_encoder_t *),
void *param);


EXPORT void obs_enum_services(bool (*enum_proc)(void *, obs_service_t *),
void *param);







EXPORT obs_source_t *obs_get_source_by_name(const char *name);


EXPORT obs_output_t *obs_get_output_by_name(const char *name);


EXPORT obs_encoder_t *obs_get_encoder_by_name(const char *name);


EXPORT obs_service_t *obs_get_service_by_name(const char *name);

enum obs_base_effect {
OBS_EFFECT_DEFAULT, 
OBS_EFFECT_DEFAULT_RECT, 
OBS_EFFECT_OPAQUE, 
OBS_EFFECT_SOLID, 
OBS_EFFECT_BICUBIC, 
OBS_EFFECT_LANCZOS, 
OBS_EFFECT_BILINEAR_LOWRES, 
OBS_EFFECT_PREMULTIPLIED_ALPHA, 
OBS_EFFECT_REPEAT, 
OBS_EFFECT_AREA, 
};


EXPORT gs_effect_t *obs_get_base_effect(enum obs_base_effect effect);

#if !defined(SWIG)

OBS_DEPRECATED
EXPORT gs_effect_t *obs_get_default_rect_effect(void);
#endif


EXPORT signal_handler_t *obs_get_signal_handler(void);


EXPORT proc_handler_t *obs_get_proc_handler(void);

#if !defined(SWIG)

OBS_DEPRECATED
EXPORT void obs_render_main_view(void);
#endif


EXPORT void obs_render_main_texture(void);


EXPORT void obs_render_main_texture_src_color_only(void);



EXPORT gs_texture_t *obs_get_main_texture(void);


EXPORT void obs_set_master_volume(float volume);


EXPORT float obs_get_master_volume(void);


EXPORT obs_data_t *obs_save_source(obs_source_t *source);


EXPORT obs_source_t *obs_load_source(obs_data_t *data);


EXPORT void obs_source_save(obs_source_t *source);


EXPORT void obs_source_load(obs_source_t *source);

typedef void (*obs_load_source_cb)(void *private_data, obs_source_t *source);


EXPORT void obs_load_sources(obs_data_array_t *array, obs_load_source_cb cb,
void *private_data);


EXPORT obs_data_array_t *obs_save_sources(void);

typedef bool (*obs_save_source_filter_cb)(void *data, obs_source_t *source);
EXPORT obs_data_array_t *obs_save_sources_filtered(obs_save_source_filter_cb cb,
void *data);

enum obs_obj_type {
OBS_OBJ_TYPE_INVALID,
OBS_OBJ_TYPE_SOURCE,
OBS_OBJ_TYPE_OUTPUT,
OBS_OBJ_TYPE_ENCODER,
OBS_OBJ_TYPE_SERVICE,
};

EXPORT enum obs_obj_type obs_obj_get_type(void *obj);
EXPORT const char *obs_obj_get_id(void *obj);
EXPORT bool obs_obj_invalid(void *obj);
EXPORT void *obs_obj_get_data(void *obj);

typedef bool (*obs_enum_audio_device_cb)(void *data, const char *name,
const char *id);

EXPORT void obs_enum_audio_monitoring_devices(obs_enum_audio_device_cb cb,
void *data);

EXPORT bool obs_set_audio_monitoring_device(const char *name, const char *id);
EXPORT void obs_get_audio_monitoring_device(const char **name, const char **id);

EXPORT void obs_add_tick_callback(void (*tick)(void *param, float seconds),
void *param);
EXPORT void obs_remove_tick_callback(void (*tick)(void *param, float seconds),
void *param);

EXPORT void obs_add_main_render_callback(void (*draw)(void *param, uint32_t cx,
uint32_t cy),
void *param);
EXPORT void obs_remove_main_render_callback(
void (*draw)(void *param, uint32_t cx, uint32_t cy), void *param);

EXPORT void obs_add_raw_video_callback(
const struct video_scale_info *conversion,
void (*callback)(void *param, struct video_data *frame), void *param);
EXPORT void obs_remove_raw_video_callback(
void (*callback)(void *param, struct video_data *frame), void *param);

EXPORT uint64_t obs_get_video_frame_time(void);

EXPORT double obs_get_active_fps(void);
EXPORT uint64_t obs_get_average_frame_time_ns(void);
EXPORT uint64_t obs_get_frame_interval_ns(void);

EXPORT uint32_t obs_get_total_frames(void);
EXPORT uint32_t obs_get_lagged_frames(void);

EXPORT bool obs_nv12_tex_active(void);

EXPORT void obs_apply_private_data(obs_data_t *settings);
EXPORT void obs_set_private_data(obs_data_t *settings);
EXPORT obs_data_t *obs_get_private_data(void);

typedef void (*obs_task_t)(void *param);

enum obs_task_type {
OBS_TASK_UI,
OBS_TASK_GRAPHICS,
};

EXPORT void obs_queue_task(enum obs_task_type type, obs_task_t task,
void *param, bool wait);

typedef void (*obs_task_handler_t)(obs_task_t task, void *param, bool wait);
EXPORT void obs_set_ui_task_handler(obs_task_handler_t handler);










EXPORT obs_view_t *obs_view_create(void);


EXPORT void obs_view_destroy(obs_view_t *view);


EXPORT void obs_view_set_source(obs_view_t *view, uint32_t channel,
obs_source_t *source);


EXPORT obs_source_t *obs_view_get_source(obs_view_t *view, uint32_t channel);


EXPORT void obs_view_render(obs_view_t *view);











EXPORT obs_display_t *
obs_display_create(const struct gs_init_data *graphics_data,
uint32_t backround_color);


EXPORT void obs_display_destroy(obs_display_t *display);


EXPORT void obs_display_resize(obs_display_t *display, uint32_t cx,
uint32_t cy);









EXPORT void obs_display_add_draw_callback(obs_display_t *display,
void (*draw)(void *param, uint32_t cx,
uint32_t cy),
void *param);


EXPORT void obs_display_remove_draw_callback(
obs_display_t *display,
void (*draw)(void *param, uint32_t cx, uint32_t cy), void *param);

EXPORT void obs_display_set_enabled(obs_display_t *display, bool enable);
EXPORT bool obs_display_enabled(obs_display_t *display);

EXPORT void obs_display_set_background_color(obs_display_t *display,
uint32_t color);

EXPORT void obs_display_size(obs_display_t *display, uint32_t *width,
uint32_t *height);





EXPORT const char *obs_source_get_display_name(const char *id);







EXPORT obs_source_t *obs_source_create(const char *id, const char *name,
obs_data_t *settings,
obs_data_t *hotkey_data);

EXPORT obs_source_t *obs_source_create_private(const char *id, const char *name,
obs_data_t *settings);



EXPORT obs_source_t *obs_source_duplicate(obs_source_t *source,
const char *desired_name,
bool create_private);




EXPORT void obs_source_addref(obs_source_t *source);
EXPORT void obs_source_release(obs_source_t *source);

EXPORT void obs_weak_source_addref(obs_weak_source_t *weak);
EXPORT void obs_weak_source_release(obs_weak_source_t *weak);

EXPORT obs_source_t *obs_source_get_ref(obs_source_t *source);
EXPORT obs_weak_source_t *obs_source_get_weak_source(obs_source_t *source);
EXPORT obs_source_t *obs_weak_source_get_source(obs_weak_source_t *weak);

EXPORT bool obs_weak_source_references_source(obs_weak_source_t *weak,
obs_source_t *source);


EXPORT void obs_source_remove(obs_source_t *source);


EXPORT bool obs_source_removed(const obs_source_t *source);


EXPORT uint32_t obs_source_get_output_flags(const obs_source_t *source);


EXPORT uint32_t obs_get_source_output_flags(const char *id);


EXPORT obs_data_t *obs_get_source_defaults(const char *id);


EXPORT obs_properties_t *obs_get_source_properties(const char *id);


EXPORT bool obs_is_source_configurable(const char *id);

EXPORT bool obs_source_configurable(const obs_source_t *source);





EXPORT obs_properties_t *obs_source_properties(const obs_source_t *source);


EXPORT void obs_source_update(obs_source_t *source, obs_data_t *settings);


EXPORT void obs_source_video_render(obs_source_t *source);


EXPORT uint32_t obs_source_get_width(obs_source_t *source);


EXPORT uint32_t obs_source_get_height(obs_source_t *source);






EXPORT obs_source_t *obs_filter_get_parent(const obs_source_t *filter);






EXPORT obs_source_t *obs_filter_get_target(const obs_source_t *filter);


EXPORT void obs_source_default_render(obs_source_t *source);


EXPORT void obs_source_filter_add(obs_source_t *source, obs_source_t *filter);


EXPORT void obs_source_filter_remove(obs_source_t *source,
obs_source_t *filter);


EXPORT void obs_source_filter_set_order(obs_source_t *source,
obs_source_t *filter,
enum obs_order_movement movement);


EXPORT obs_data_t *obs_source_get_settings(const obs_source_t *source);


EXPORT const char *obs_source_get_name(const obs_source_t *source);


EXPORT void obs_source_set_name(obs_source_t *source, const char *name);


EXPORT enum obs_source_type obs_source_get_type(const obs_source_t *source);


EXPORT const char *obs_source_get_id(const obs_source_t *source);
EXPORT const char *obs_source_get_unversioned_id(const obs_source_t *source);


EXPORT signal_handler_t *
obs_source_get_signal_handler(const obs_source_t *source);


EXPORT proc_handler_t *obs_source_get_proc_handler(const obs_source_t *source);


EXPORT void obs_source_set_volume(obs_source_t *source, float volume);


EXPORT float obs_source_get_volume(const obs_source_t *source);


EXPORT enum speaker_layout obs_source_get_speaker_layout(obs_source_t *source);


EXPORT void obs_source_set_balance_value(obs_source_t *source, float balance);


EXPORT float obs_source_get_balance_value(const obs_source_t *source);


EXPORT void obs_source_set_sync_offset(obs_source_t *source, int64_t offset);


EXPORT int64_t obs_source_get_sync_offset(const obs_source_t *source);


EXPORT void obs_source_enum_active_sources(obs_source_t *source,
obs_source_enum_proc_t enum_callback,
void *param);


EXPORT void obs_source_enum_active_tree(obs_source_t *source,
obs_source_enum_proc_t enum_callback,
void *param);


EXPORT bool obs_source_active(const obs_source_t *source);




EXPORT bool obs_source_showing(const obs_source_t *source);


#define OBS_SOURCE_FLAG_UNUSED_1 (1 << 0)

#define OBS_SOURCE_FLAG_FORCE_MONO (1 << 1)






EXPORT void obs_source_set_flags(obs_source_t *source, uint32_t flags);


EXPORT uint32_t obs_source_get_flags(const obs_source_t *source);





EXPORT void obs_source_set_audio_mixers(obs_source_t *source, uint32_t mixers);


EXPORT uint32_t obs_source_get_audio_mixers(const obs_source_t *source);






EXPORT void obs_source_inc_showing(obs_source_t *source);











EXPORT void obs_source_inc_active(obs_source_t *source);






EXPORT void obs_source_dec_showing(obs_source_t *source);










EXPORT void obs_source_dec_active(obs_source_t *source);


EXPORT void obs_source_enum_filters(obs_source_t *source,
obs_source_enum_proc_t callback,
void *param);


EXPORT obs_source_t *obs_source_get_filter_by_name(obs_source_t *source,
const char *name);

EXPORT void obs_source_copy_filters(obs_source_t *dst, obs_source_t *src);

EXPORT bool obs_source_enabled(const obs_source_t *source);
EXPORT void obs_source_set_enabled(obs_source_t *source, bool enabled);

EXPORT bool obs_source_muted(const obs_source_t *source);
EXPORT void obs_source_set_muted(obs_source_t *source, bool muted);

EXPORT bool obs_source_push_to_mute_enabled(obs_source_t *source);
EXPORT void obs_source_enable_push_to_mute(obs_source_t *source, bool enabled);

EXPORT uint64_t obs_source_get_push_to_mute_delay(obs_source_t *source);
EXPORT void obs_source_set_push_to_mute_delay(obs_source_t *source,
uint64_t delay);

EXPORT bool obs_source_push_to_talk_enabled(obs_source_t *source);
EXPORT void obs_source_enable_push_to_talk(obs_source_t *source, bool enabled);

EXPORT uint64_t obs_source_get_push_to_talk_delay(obs_source_t *source);
EXPORT void obs_source_set_push_to_talk_delay(obs_source_t *source,
uint64_t delay);

typedef void (*obs_source_audio_capture_t)(void *param, obs_source_t *source,
const struct audio_data *audio_data,
bool muted);

EXPORT void obs_source_add_audio_capture_callback(
obs_source_t *source, obs_source_audio_capture_t callback, void *param);
EXPORT void obs_source_remove_audio_capture_callback(
obs_source_t *source, obs_source_audio_capture_t callback, void *param);

enum obs_deinterlace_mode {
OBS_DEINTERLACE_MODE_DISABLE,
OBS_DEINTERLACE_MODE_DISCARD,
OBS_DEINTERLACE_MODE_RETRO,
OBS_DEINTERLACE_MODE_BLEND,
OBS_DEINTERLACE_MODE_BLEND_2X,
OBS_DEINTERLACE_MODE_LINEAR,
OBS_DEINTERLACE_MODE_LINEAR_2X,
OBS_DEINTERLACE_MODE_YADIF,
OBS_DEINTERLACE_MODE_YADIF_2X,
};

enum obs_deinterlace_field_order {
OBS_DEINTERLACE_FIELD_ORDER_TOP,
OBS_DEINTERLACE_FIELD_ORDER_BOTTOM,
};

EXPORT void obs_source_set_deinterlace_mode(obs_source_t *source,
enum obs_deinterlace_mode mode);
EXPORT enum obs_deinterlace_mode
obs_source_get_deinterlace_mode(const obs_source_t *source);
EXPORT void obs_source_set_deinterlace_field_order(
obs_source_t *source, enum obs_deinterlace_field_order field_order);
EXPORT enum obs_deinterlace_field_order
obs_source_get_deinterlace_field_order(const obs_source_t *source);

enum obs_monitoring_type {
OBS_MONITORING_TYPE_NONE,
OBS_MONITORING_TYPE_MONITOR_ONLY,
OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT,
};

EXPORT void obs_source_set_monitoring_type(obs_source_t *source,
enum obs_monitoring_type type);
EXPORT enum obs_monitoring_type
obs_source_get_monitoring_type(const obs_source_t *source);



EXPORT obs_data_t *obs_source_get_private_settings(obs_source_t *item);




EXPORT void *obs_source_get_type_data(obs_source_t *source);













EXPORT void
obs_source_draw_set_color_matrix(const struct matrix4 *color_matrix,
const struct vec3 *color_range_min,
const struct vec3 *color_range_max);












EXPORT void obs_source_draw(gs_texture_t *image, int x, int y, uint32_t cx,
uint32_t cy, bool flip);








EXPORT void obs_source_output_video(obs_source_t *source,
const struct obs_source_frame *frame);
EXPORT void obs_source_output_video2(obs_source_t *source,
const struct obs_source_frame2 *frame);

EXPORT void obs_source_set_async_rotation(obs_source_t *source, long rotation);








EXPORT void obs_source_preload_video(obs_source_t *source,
const struct obs_source_frame *frame);
EXPORT void obs_source_preload_video2(obs_source_t *source,
const struct obs_source_frame2 *frame);


EXPORT void obs_source_show_preloaded_video(obs_source_t *source);


EXPORT void obs_source_output_audio(obs_source_t *source,
const struct obs_source_audio *audio);


EXPORT void obs_source_update_properties(obs_source_t *source);


EXPORT struct obs_source_frame *obs_source_get_frame(obs_source_t *source);


EXPORT void obs_source_release_frame(obs_source_t *source,
struct obs_source_frame *frame);












EXPORT bool
obs_source_process_filter_begin(obs_source_t *filter,
enum gs_color_format format,
enum obs_allow_direct_render allow_direct);








EXPORT void obs_source_process_filter_end(obs_source_t *filter,
gs_effect_t *effect, uint32_t width,
uint32_t height);








EXPORT void obs_source_process_filter_tech_end(obs_source_t *filter,
gs_effect_t *effect,
uint32_t width, uint32_t height,
const char *tech_name);


EXPORT void obs_source_skip_video_filter(obs_source_t *filter);








EXPORT bool obs_source_add_active_child(obs_source_t *parent,
obs_source_t *child);






EXPORT void obs_source_remove_active_child(obs_source_t *parent,
obs_source_t *child);


EXPORT void obs_source_send_mouse_click(obs_source_t *source,
const struct obs_mouse_event *event,
int32_t type, bool mouse_up,
uint32_t click_count);


EXPORT void obs_source_send_mouse_move(obs_source_t *source,
const struct obs_mouse_event *event,
bool mouse_leave);


EXPORT void obs_source_send_mouse_wheel(obs_source_t *source,
const struct obs_mouse_event *event,
int x_delta, int y_delta);


EXPORT void obs_source_send_focus(obs_source_t *source, bool focus);


EXPORT void obs_source_send_key_click(obs_source_t *source,
const struct obs_key_event *event,
bool key_up);


EXPORT void obs_source_set_default_flags(obs_source_t *source, uint32_t flags);


EXPORT uint32_t obs_source_get_base_width(obs_source_t *source);


EXPORT uint32_t obs_source_get_base_height(obs_source_t *source);

EXPORT bool obs_source_audio_pending(const obs_source_t *source);
EXPORT uint64_t obs_source_get_audio_timestamp(const obs_source_t *source);
EXPORT void obs_source_get_audio_mix(const obs_source_t *source,
struct obs_source_audio_mix *audio);

EXPORT void obs_source_set_async_unbuffered(obs_source_t *source,
bool unbuffered);
EXPORT bool obs_source_async_unbuffered(const obs_source_t *source);




EXPORT void obs_source_set_async_decoupled(obs_source_t *source, bool decouple);
EXPORT bool obs_source_async_decoupled(const obs_source_t *source);

EXPORT void obs_source_set_audio_active(obs_source_t *source, bool show);
EXPORT bool obs_source_audio_active(const obs_source_t *source);

EXPORT uint32_t obs_source_get_last_obs_version(const obs_source_t *source);


EXPORT void obs_source_media_play_pause(obs_source_t *source, bool pause);
EXPORT void obs_source_media_restart(obs_source_t *source);
EXPORT void obs_source_media_stop(obs_source_t *source);
EXPORT void obs_source_media_next(obs_source_t *source);
EXPORT void obs_source_media_previous(obs_source_t *source);
EXPORT int64_t obs_source_media_get_duration(obs_source_t *source);
EXPORT int64_t obs_source_media_get_time(obs_source_t *source);
EXPORT void obs_source_media_set_time(obs_source_t *source, int64_t ms);
EXPORT enum obs_media_state obs_source_media_get_state(obs_source_t *source);
EXPORT void obs_source_media_started(obs_source_t *source);
EXPORT void obs_source_media_ended(obs_source_t *source);



enum obs_transition_target {
OBS_TRANSITION_SOURCE_A,
OBS_TRANSITION_SOURCE_B,
};

EXPORT obs_source_t *
obs_transition_get_source(obs_source_t *transition,
enum obs_transition_target target);
EXPORT void obs_transition_clear(obs_source_t *transition);

EXPORT obs_source_t *obs_transition_get_active_source(obs_source_t *transition);

enum obs_transition_mode {
OBS_TRANSITION_MODE_AUTO,
OBS_TRANSITION_MODE_MANUAL,
};

EXPORT bool obs_transition_start(obs_source_t *transition,
enum obs_transition_mode mode,
uint32_t duration_ms, obs_source_t *dest);

EXPORT void obs_transition_set(obs_source_t *transition, obs_source_t *source);

EXPORT void obs_transition_set_manual_time(obs_source_t *transition, float t);
EXPORT void obs_transition_set_manual_torque(obs_source_t *transition,
float torque, float clamp);

enum obs_transition_scale_type {
OBS_TRANSITION_SCALE_MAX_ONLY,
OBS_TRANSITION_SCALE_ASPECT,
OBS_TRANSITION_SCALE_STRETCH,
};

EXPORT void obs_transition_set_scale_type(obs_source_t *transition,
enum obs_transition_scale_type type);
EXPORT enum obs_transition_scale_type
obs_transition_get_scale_type(const obs_source_t *transition);

EXPORT void obs_transition_set_alignment(obs_source_t *transition,
uint32_t alignment);
EXPORT uint32_t obs_transition_get_alignment(const obs_source_t *transition);

EXPORT void obs_transition_set_size(obs_source_t *transition, uint32_t cx,
uint32_t cy);
EXPORT void obs_transition_get_size(const obs_source_t *transition,
uint32_t *cx, uint32_t *cy);







EXPORT void obs_transition_enable_fixed(obs_source_t *transition, bool enable,
uint32_t duration_ms);
EXPORT bool obs_transition_fixed(obs_source_t *transition);

typedef void (*obs_transition_video_render_callback_t)(void *data,
gs_texture_t *a,
gs_texture_t *b, float t,
uint32_t cx,
uint32_t cy);
typedef float (*obs_transition_audio_mix_callback_t)(void *data, float t);

EXPORT float obs_transition_get_time(obs_source_t *transition);

EXPORT void obs_transition_force_stop(obs_source_t *transition);

EXPORT void
obs_transition_video_render(obs_source_t *transition,
obs_transition_video_render_callback_t callback);



EXPORT bool
obs_transition_video_render_direct(obs_source_t *transition,
enum obs_transition_target target);

EXPORT bool
obs_transition_audio_render(obs_source_t *transition, uint64_t *ts_out,
struct obs_source_audio_mix *audio, uint32_t mixers,
size_t channels, size_t sample_rate,
obs_transition_audio_mix_callback_t mix_a_callback,
obs_transition_audio_mix_callback_t mix_b_callback);



EXPORT void obs_transition_swap_begin(obs_source_t *tr_dest,
obs_source_t *tr_source);
EXPORT void obs_transition_swap_end(obs_source_t *tr_dest,
obs_source_t *tr_source);










EXPORT obs_scene_t *obs_scene_create(const char *name);

EXPORT obs_scene_t *obs_scene_create_private(const char *name);

enum obs_scene_duplicate_type {
OBS_SCENE_DUP_REFS, 
OBS_SCENE_DUP_COPY, 
OBS_SCENE_DUP_PRIVATE_REFS, 
OBS_SCENE_DUP_PRIVATE_COPY, 
};




EXPORT obs_scene_t *obs_scene_duplicate(obs_scene_t *scene, const char *name,
enum obs_scene_duplicate_type type);

EXPORT void obs_scene_addref(obs_scene_t *scene);
EXPORT void obs_scene_release(obs_scene_t *scene);


EXPORT obs_source_t *obs_scene_get_source(const obs_scene_t *scene);


EXPORT obs_scene_t *obs_scene_from_source(const obs_source_t *source);


EXPORT obs_sceneitem_t *obs_scene_find_source(obs_scene_t *scene,
const char *name);

EXPORT obs_sceneitem_t *obs_scene_find_source_recursive(obs_scene_t *scene,
const char *name);

EXPORT obs_sceneitem_t *obs_scene_find_sceneitem_by_id(obs_scene_t *scene,
int64_t id);


EXPORT void obs_scene_enum_items(obs_scene_t *scene,
bool (*callback)(obs_scene_t *,
obs_sceneitem_t *, void *),
void *param);

EXPORT bool obs_scene_reorder_items(obs_scene_t *scene,
obs_sceneitem_t *const *item_order,
size_t item_order_size);

struct obs_sceneitem_order_info {
obs_sceneitem_t *group;
obs_sceneitem_t *item;
};

EXPORT bool
obs_scene_reorder_items2(obs_scene_t *scene,
struct obs_sceneitem_order_info *item_order,
size_t item_order_size);


EXPORT obs_sceneitem_t *obs_scene_add(obs_scene_t *scene, obs_source_t *source);

typedef void (*obs_scene_atomic_update_func)(void *, obs_scene_t *scene);
EXPORT void obs_scene_atomic_update(obs_scene_t *scene,
obs_scene_atomic_update_func func,
void *data);

EXPORT void obs_sceneitem_addref(obs_sceneitem_t *item);
EXPORT void obs_sceneitem_release(obs_sceneitem_t *item);


EXPORT void obs_sceneitem_remove(obs_sceneitem_t *item);


EXPORT obs_scene_t *obs_sceneitem_get_scene(const obs_sceneitem_t *item);


EXPORT obs_source_t *obs_sceneitem_get_source(const obs_sceneitem_t *item);



EXPORT void obs_sceneitem_select(obs_sceneitem_t *item, bool select);
EXPORT bool obs_sceneitem_selected(const obs_sceneitem_t *item);
EXPORT bool obs_sceneitem_locked(const obs_sceneitem_t *item);
EXPORT bool obs_sceneitem_set_locked(obs_sceneitem_t *item, bool lock);


EXPORT void obs_sceneitem_set_pos(obs_sceneitem_t *item,
const struct vec2 *pos);
EXPORT void obs_sceneitem_set_rot(obs_sceneitem_t *item, float rot_deg);
EXPORT void obs_sceneitem_set_scale(obs_sceneitem_t *item,
const struct vec2 *scale);
EXPORT void obs_sceneitem_set_alignment(obs_sceneitem_t *item,
uint32_t alignment);
EXPORT void obs_sceneitem_set_order(obs_sceneitem_t *item,
enum obs_order_movement movement);
EXPORT void obs_sceneitem_set_order_position(obs_sceneitem_t *item,
int position);
EXPORT void obs_sceneitem_set_bounds_type(obs_sceneitem_t *item,
enum obs_bounds_type type);
EXPORT void obs_sceneitem_set_bounds_alignment(obs_sceneitem_t *item,
uint32_t alignment);
EXPORT void obs_sceneitem_set_bounds(obs_sceneitem_t *item,
const struct vec2 *bounds);

EXPORT int64_t obs_sceneitem_get_id(const obs_sceneitem_t *item);

EXPORT void obs_sceneitem_get_pos(const obs_sceneitem_t *item,
struct vec2 *pos);
EXPORT float obs_sceneitem_get_rot(const obs_sceneitem_t *item);
EXPORT void obs_sceneitem_get_scale(const obs_sceneitem_t *item,
struct vec2 *scale);
EXPORT uint32_t obs_sceneitem_get_alignment(const obs_sceneitem_t *item);

EXPORT enum obs_bounds_type
obs_sceneitem_get_bounds_type(const obs_sceneitem_t *item);
EXPORT uint32_t obs_sceneitem_get_bounds_alignment(const obs_sceneitem_t *item);
EXPORT void obs_sceneitem_get_bounds(const obs_sceneitem_t *item,
struct vec2 *bounds);

EXPORT void obs_sceneitem_get_info(const obs_sceneitem_t *item,
struct obs_transform_info *info);
EXPORT void obs_sceneitem_set_info(obs_sceneitem_t *item,
const struct obs_transform_info *info);

EXPORT void obs_sceneitem_get_draw_transform(const obs_sceneitem_t *item,
struct matrix4 *transform);
EXPORT void obs_sceneitem_get_box_transform(const obs_sceneitem_t *item,
struct matrix4 *transform);
EXPORT void obs_sceneitem_get_box_scale(const obs_sceneitem_t *item,
struct vec2 *scale);

EXPORT bool obs_sceneitem_visible(const obs_sceneitem_t *item);
EXPORT bool obs_sceneitem_set_visible(obs_sceneitem_t *item, bool visible);

struct obs_sceneitem_crop {
int left;
int top;
int right;
int bottom;
};

EXPORT void obs_sceneitem_set_crop(obs_sceneitem_t *item,
const struct obs_sceneitem_crop *crop);
EXPORT void obs_sceneitem_get_crop(const obs_sceneitem_t *item,
struct obs_sceneitem_crop *crop);

EXPORT void obs_sceneitem_set_scale_filter(obs_sceneitem_t *item,
enum obs_scale_type filter);
EXPORT enum obs_scale_type
obs_sceneitem_get_scale_filter(obs_sceneitem_t *item);

EXPORT void obs_sceneitem_force_update_transform(obs_sceneitem_t *item);

EXPORT void obs_sceneitem_defer_update_begin(obs_sceneitem_t *item);
EXPORT void obs_sceneitem_defer_update_end(obs_sceneitem_t *item);



EXPORT obs_data_t *obs_sceneitem_get_private_settings(obs_sceneitem_t *item);

EXPORT obs_sceneitem_t *obs_scene_add_group(obs_scene_t *scene,
const char *name);
EXPORT obs_sceneitem_t *obs_scene_insert_group(obs_scene_t *scene,
const char *name,
obs_sceneitem_t **items,
size_t count);

EXPORT obs_sceneitem_t *obs_scene_add_group2(obs_scene_t *scene,
const char *name, bool signal);
EXPORT obs_sceneitem_t *obs_scene_insert_group2(obs_scene_t *scene,
const char *name,
obs_sceneitem_t **items,
size_t count, bool signal);

EXPORT obs_sceneitem_t *obs_scene_get_group(obs_scene_t *scene,
const char *name);

EXPORT bool obs_sceneitem_is_group(obs_sceneitem_t *item);

EXPORT obs_scene_t *obs_sceneitem_group_get_scene(const obs_sceneitem_t *group);

EXPORT void obs_sceneitem_group_ungroup(obs_sceneitem_t *group);
EXPORT void obs_sceneitem_group_ungroup2(obs_sceneitem_t *group, bool signal);

EXPORT void obs_sceneitem_group_add_item(obs_sceneitem_t *group,
obs_sceneitem_t *item);
EXPORT void obs_sceneitem_group_remove_item(obs_sceneitem_t *group,
obs_sceneitem_t *item);

EXPORT obs_sceneitem_t *obs_sceneitem_get_group(obs_scene_t *scene,
obs_sceneitem_t *item);

EXPORT bool obs_source_is_group(const obs_source_t *source);
EXPORT bool obs_scene_is_group(const obs_scene_t *scene);

EXPORT void obs_sceneitem_group_enum_items(obs_sceneitem_t *group,
bool (*callback)(obs_scene_t *,
obs_sceneitem_t *,
void *),
void *param);


EXPORT obs_scene_t *obs_group_from_source(const obs_source_t *source);

EXPORT void obs_sceneitem_defer_group_resize_begin(obs_sceneitem_t *item);
EXPORT void obs_sceneitem_defer_group_resize_end(obs_sceneitem_t *item);




EXPORT const char *obs_output_get_display_name(const char *id);







EXPORT obs_output_t *obs_output_create(const char *id, const char *name,
obs_data_t *settings,
obs_data_t *hotkey_data);





EXPORT void obs_output_addref(obs_output_t *output);
EXPORT void obs_output_release(obs_output_t *output);

EXPORT void obs_weak_output_addref(obs_weak_output_t *weak);
EXPORT void obs_weak_output_release(obs_weak_output_t *weak);

EXPORT obs_output_t *obs_output_get_ref(obs_output_t *output);
EXPORT obs_weak_output_t *obs_output_get_weak_output(obs_output_t *output);
EXPORT obs_output_t *obs_weak_output_get_output(obs_weak_output_t *weak);

EXPORT bool obs_weak_output_references_output(obs_weak_output_t *weak,
obs_output_t *output);

EXPORT const char *obs_output_get_name(const obs_output_t *output);


EXPORT bool obs_output_start(obs_output_t *output);


EXPORT void obs_output_stop(obs_output_t *output);






#define OBS_OUTPUT_DELAY_PRESERVE (1 << 0)








EXPORT void obs_output_set_delay(obs_output_t *output, uint32_t delay_sec,
uint32_t flags);


EXPORT uint32_t obs_output_get_delay(const obs_output_t *output);


EXPORT uint32_t obs_output_get_active_delay(const obs_output_t *output);


EXPORT void obs_output_force_stop(obs_output_t *output);


EXPORT bool obs_output_active(const obs_output_t *output);


EXPORT uint32_t obs_output_get_flags(const obs_output_t *output);


EXPORT uint32_t obs_get_output_flags(const char *id);


EXPORT obs_data_t *obs_output_defaults(const char *id);


EXPORT obs_properties_t *obs_get_output_properties(const char *id);





EXPORT obs_properties_t *obs_output_properties(const obs_output_t *output);


EXPORT void obs_output_update(obs_output_t *output, obs_data_t *settings);


EXPORT bool obs_output_can_pause(const obs_output_t *output);


EXPORT bool obs_output_pause(obs_output_t *output, bool pause);


EXPORT bool obs_output_paused(const obs_output_t *output);


EXPORT obs_data_t *obs_output_get_settings(const obs_output_t *output);


EXPORT signal_handler_t *
obs_output_get_signal_handler(const obs_output_t *output);


EXPORT proc_handler_t *obs_output_get_proc_handler(const obs_output_t *output);





EXPORT void obs_output_set_media(obs_output_t *output, video_t *video,
audio_t *audio);


EXPORT video_t *obs_output_video(const obs_output_t *output);


EXPORT audio_t *obs_output_audio(const obs_output_t *output);


EXPORT void obs_output_set_mixer(obs_output_t *output, size_t mixer_idx);


EXPORT size_t obs_output_get_mixer(const obs_output_t *output);


EXPORT void obs_output_set_mixers(obs_output_t *output, size_t mixers);


EXPORT size_t obs_output_get_mixers(const obs_output_t *output);





EXPORT void obs_output_set_video_encoder(obs_output_t *output,
obs_encoder_t *encoder);









EXPORT void obs_output_set_audio_encoder(obs_output_t *output,
obs_encoder_t *encoder, size_t idx);


EXPORT obs_encoder_t *obs_output_get_video_encoder(const obs_output_t *output);








EXPORT obs_encoder_t *obs_output_get_audio_encoder(const obs_output_t *output,
size_t idx);


EXPORT void obs_output_set_service(obs_output_t *output,
obs_service_t *service);


EXPORT obs_service_t *obs_output_get_service(const obs_output_t *output);




EXPORT void obs_output_set_reconnect_settings(obs_output_t *output,
int retry_count, int retry_sec);

EXPORT uint64_t obs_output_get_total_bytes(const obs_output_t *output);
EXPORT int obs_output_get_frames_dropped(const obs_output_t *output);
EXPORT int obs_output_get_total_frames(const obs_output_t *output);









EXPORT void obs_output_set_preferred_size(obs_output_t *output, uint32_t width,
uint32_t height);


EXPORT uint32_t obs_output_get_width(const obs_output_t *output);


EXPORT uint32_t obs_output_get_height(const obs_output_t *output);

EXPORT const char *obs_output_get_id(const obs_output_t *output);

#if BUILD_CAPTIONS
EXPORT void obs_output_output_caption_text1(obs_output_t *output,
const char *text);
EXPORT void obs_output_output_caption_text2(obs_output_t *output,
const char *text,
double display_duration);
#endif

EXPORT float obs_output_get_congestion(obs_output_t *output);
EXPORT int obs_output_get_connect_time_ms(obs_output_t *output);

EXPORT bool obs_output_reconnecting(const obs_output_t *output);


EXPORT void obs_output_set_last_error(obs_output_t *output,
const char *message);
EXPORT const char *obs_output_get_last_error(obs_output_t *output);

EXPORT const char *
obs_output_get_supported_video_codecs(const obs_output_t *output);
EXPORT const char *
obs_output_get_supported_audio_codecs(const obs_output_t *output);




EXPORT void *obs_output_get_type_data(obs_output_t *output);


EXPORT void
obs_output_set_video_conversion(obs_output_t *output,
const struct video_scale_info *conversion);


EXPORT void
obs_output_set_audio_conversion(obs_output_t *output,
const struct audio_convert_info *conversion);


EXPORT bool obs_output_can_begin_data_capture(const obs_output_t *output,
uint32_t flags);


EXPORT bool obs_output_initialize_encoders(obs_output_t *output,
uint32_t flags);













EXPORT bool obs_output_begin_data_capture(obs_output_t *output, uint32_t flags);


EXPORT void obs_output_end_data_capture(obs_output_t *output);







EXPORT void obs_output_signal_stop(obs_output_t *output, int code);

EXPORT uint64_t obs_output_get_pause_offset(obs_output_t *output);




EXPORT const char *obs_encoder_get_display_name(const char *id);









EXPORT obs_encoder_t *obs_video_encoder_create(const char *id, const char *name,
obs_data_t *settings,
obs_data_t *hotkey_data);










EXPORT obs_encoder_t *obs_audio_encoder_create(const char *id, const char *name,
obs_data_t *settings,
size_t mixer_idx,
obs_data_t *hotkey_data);





EXPORT void obs_encoder_addref(obs_encoder_t *encoder);
EXPORT void obs_encoder_release(obs_encoder_t *encoder);

EXPORT void obs_weak_encoder_addref(obs_weak_encoder_t *weak);
EXPORT void obs_weak_encoder_release(obs_weak_encoder_t *weak);

EXPORT obs_encoder_t *obs_encoder_get_ref(obs_encoder_t *encoder);
EXPORT obs_weak_encoder_t *obs_encoder_get_weak_encoder(obs_encoder_t *encoder);
EXPORT obs_encoder_t *obs_weak_encoder_get_encoder(obs_weak_encoder_t *weak);

EXPORT bool obs_weak_encoder_references_encoder(obs_weak_encoder_t *weak,
obs_encoder_t *encoder);

EXPORT void obs_encoder_set_name(obs_encoder_t *encoder, const char *name);
EXPORT const char *obs_encoder_get_name(const obs_encoder_t *encoder);


EXPORT const char *obs_get_encoder_codec(const char *id);


EXPORT enum obs_encoder_type obs_get_encoder_type(const char *id);


EXPORT const char *obs_encoder_get_codec(const obs_encoder_t *encoder);


EXPORT enum obs_encoder_type obs_encoder_get_type(const obs_encoder_t *encoder);






EXPORT void obs_encoder_set_scaled_size(obs_encoder_t *encoder, uint32_t width,
uint32_t height);


EXPORT bool obs_encoder_scaling_enabled(const obs_encoder_t *encoder);


EXPORT uint32_t obs_encoder_get_width(const obs_encoder_t *encoder);


EXPORT uint32_t obs_encoder_get_height(const obs_encoder_t *encoder);


EXPORT uint32_t obs_encoder_get_sample_rate(const obs_encoder_t *encoder);









EXPORT void obs_encoder_set_preferred_video_format(obs_encoder_t *encoder,
enum video_format format);
EXPORT enum video_format
obs_encoder_get_preferred_video_format(const obs_encoder_t *encoder);


EXPORT obs_data_t *obs_encoder_defaults(const char *id);
EXPORT obs_data_t *obs_encoder_get_defaults(const obs_encoder_t *encoder);


EXPORT obs_properties_t *obs_get_encoder_properties(const char *id);





EXPORT obs_properties_t *obs_encoder_properties(const obs_encoder_t *encoder);





EXPORT void obs_encoder_update(obs_encoder_t *encoder, obs_data_t *settings);


EXPORT bool obs_encoder_get_extra_data(const obs_encoder_t *encoder,
uint8_t **extra_data, size_t *size);


EXPORT obs_data_t *obs_encoder_get_settings(const obs_encoder_t *encoder);


EXPORT void obs_encoder_set_video(obs_encoder_t *encoder, video_t *video);


EXPORT void obs_encoder_set_audio(obs_encoder_t *encoder, audio_t *audio);





EXPORT video_t *obs_encoder_video(const obs_encoder_t *encoder);





EXPORT audio_t *obs_encoder_audio(const obs_encoder_t *encoder);


EXPORT bool obs_encoder_active(const obs_encoder_t *encoder);

EXPORT void *obs_encoder_get_type_data(obs_encoder_t *encoder);

EXPORT const char *obs_encoder_get_id(const obs_encoder_t *encoder);

EXPORT uint32_t obs_get_encoder_caps(const char *encoder_id);
EXPORT uint32_t obs_encoder_get_caps(const obs_encoder_t *encoder);

#if !defined(SWIG)

OBS_DEPRECATED
EXPORT void obs_duplicate_encoder_packet(struct encoder_packet *dst,
const struct encoder_packet *src);

OBS_DEPRECATED
EXPORT void obs_free_encoder_packet(struct encoder_packet *packet);
#endif

EXPORT void obs_encoder_packet_ref(struct encoder_packet *dst,
struct encoder_packet *src);
EXPORT void obs_encoder_packet_release(struct encoder_packet *packet);

EXPORT void *obs_encoder_create_rerouted(obs_encoder_t *encoder,
const char *reroute_id);


EXPORT bool obs_encoder_paused(const obs_encoder_t *output);




EXPORT const char *obs_service_get_display_name(const char *id);

EXPORT obs_service_t *obs_service_create(const char *id, const char *name,
obs_data_t *settings,
obs_data_t *hotkey_data);

EXPORT obs_service_t *obs_service_create_private(const char *id,
const char *name,
obs_data_t *settings);





EXPORT void obs_service_addref(obs_service_t *service);
EXPORT void obs_service_release(obs_service_t *service);

EXPORT void obs_weak_service_addref(obs_weak_service_t *weak);
EXPORT void obs_weak_service_release(obs_weak_service_t *weak);

EXPORT obs_service_t *obs_service_get_ref(obs_service_t *service);
EXPORT obs_weak_service_t *obs_service_get_weak_service(obs_service_t *service);
EXPORT obs_service_t *obs_weak_service_get_service(obs_weak_service_t *weak);

EXPORT bool obs_weak_service_references_service(obs_weak_service_t *weak,
obs_service_t *service);

EXPORT const char *obs_service_get_name(const obs_service_t *service);


EXPORT obs_data_t *obs_service_defaults(const char *id);


EXPORT obs_properties_t *obs_get_service_properties(const char *id);





EXPORT obs_properties_t *obs_service_properties(const obs_service_t *service);


EXPORT const char *obs_service_get_type(const obs_service_t *service);


EXPORT void obs_service_update(obs_service_t *service, obs_data_t *settings);


EXPORT obs_data_t *obs_service_get_settings(const obs_service_t *service);


EXPORT const char *obs_service_get_url(const obs_service_t *service);


EXPORT const char *obs_service_get_key(const obs_service_t *service);


EXPORT const char *obs_service_get_username(const obs_service_t *service);


EXPORT const char *obs_service_get_password(const obs_service_t *service);







EXPORT void
obs_service_apply_encoder_settings(obs_service_t *service,
obs_data_t *video_encoder_settings,
obs_data_t *audio_encoder_settings);

EXPORT void *obs_service_get_type_data(obs_service_t *service);

EXPORT const char *obs_service_get_id(const obs_service_t *service);



EXPORT const char *obs_service_get_output_type(const obs_service_t *service);



EXPORT void obs_source_frame_init(struct obs_source_frame *frame,
enum video_format format, uint32_t width,
uint32_t height);

static inline void obs_source_frame_free(struct obs_source_frame *frame)
{
if (frame) {
bfree(frame->data[0]);
memset(frame, 0, sizeof(*frame));
}
}

static inline struct obs_source_frame *
obs_source_frame_create(enum video_format format, uint32_t width,
uint32_t height)
{
struct obs_source_frame *frame;

frame = (struct obs_source_frame *)bzalloc(sizeof(*frame));
obs_source_frame_init(frame, format, width, height);
return frame;
}

static inline void obs_source_frame_destroy(struct obs_source_frame *frame)
{
if (frame) {
bfree(frame->data[0]);
bfree(frame);
}
}

EXPORT void obs_source_frame_copy(struct obs_source_frame *dst,
const struct obs_source_frame *src);



EXPORT enum obs_icon_type obs_source_get_icon_type(const char *id);

#if defined(__cplusplus)
}
#endif
