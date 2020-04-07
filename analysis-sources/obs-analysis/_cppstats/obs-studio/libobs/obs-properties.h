#pragma once
#include "util/c99defs.h"
#include "obs-data.h"
#include "media-io/frame-rate.h"
#if defined(__cplusplus)
extern "C" {
#endif
#define OBS_PROPERTIES_DEFER_UPDATE (1 << 0)
enum obs_property_type {
OBS_PROPERTY_INVALID,
OBS_PROPERTY_BOOL,
OBS_PROPERTY_INT,
OBS_PROPERTY_FLOAT,
OBS_PROPERTY_TEXT,
OBS_PROPERTY_PATH,
OBS_PROPERTY_LIST,
OBS_PROPERTY_COLOR,
OBS_PROPERTY_BUTTON,
OBS_PROPERTY_FONT,
OBS_PROPERTY_EDITABLE_LIST,
OBS_PROPERTY_FRAME_RATE,
OBS_PROPERTY_GROUP,
};
enum obs_combo_format {
OBS_COMBO_FORMAT_INVALID,
OBS_COMBO_FORMAT_INT,
OBS_COMBO_FORMAT_FLOAT,
OBS_COMBO_FORMAT_STRING,
};
enum obs_combo_type {
OBS_COMBO_TYPE_INVALID,
OBS_COMBO_TYPE_EDITABLE,
OBS_COMBO_TYPE_LIST,
};
enum obs_editable_list_type {
OBS_EDITABLE_LIST_TYPE_STRINGS,
OBS_EDITABLE_LIST_TYPE_FILES,
OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
};
enum obs_path_type {
OBS_PATH_FILE,
OBS_PATH_FILE_SAVE,
OBS_PATH_DIRECTORY,
};
enum obs_text_type {
OBS_TEXT_DEFAULT,
OBS_TEXT_PASSWORD,
OBS_TEXT_MULTILINE,
};
enum obs_number_type {
OBS_NUMBER_SCROLLER,
OBS_NUMBER_SLIDER,
};
enum obs_group_type {
OBS_COMBO_INVALID,
OBS_GROUP_NORMAL,
OBS_GROUP_CHECKABLE,
};
#define OBS_FONT_BOLD (1 << 0)
#define OBS_FONT_ITALIC (1 << 1)
#define OBS_FONT_UNDERLINE (1 << 2)
#define OBS_FONT_STRIKEOUT (1 << 3)
struct obs_properties;
struct obs_property;
typedef struct obs_properties obs_properties_t;
typedef struct obs_property obs_property_t;
EXPORT obs_properties_t *obs_properties_create(void);
EXPORT obs_properties_t *
obs_properties_create_param(void *param, void (*destroy)(void *param));
EXPORT void obs_properties_destroy(obs_properties_t *props);
EXPORT void obs_properties_set_flags(obs_properties_t *props, uint32_t flags);
EXPORT uint32_t obs_properties_get_flags(obs_properties_t *props);
EXPORT void obs_properties_set_param(obs_properties_t *props, void *param,
void (*destroy)(void *param));
EXPORT void *obs_properties_get_param(obs_properties_t *props);
EXPORT obs_property_t *obs_properties_first(obs_properties_t *props);
EXPORT obs_property_t *obs_properties_get(obs_properties_t *props,
const char *property);
EXPORT obs_properties_t *obs_properties_get_parent(obs_properties_t *props);
EXPORT void obs_properties_remove_by_name(obs_properties_t *props,
const char *property);
EXPORT void obs_properties_apply_settings(obs_properties_t *props,
obs_data_t *settings);
typedef bool (*obs_property_clicked_t)(obs_properties_t *props,
obs_property_t *property, void *data);
EXPORT obs_property_t *obs_properties_add_bool(obs_properties_t *props,
const char *name,
const char *description);
EXPORT obs_property_t *obs_properties_add_int(obs_properties_t *props,
const char *name,
const char *description, int min,
int max, int step);
EXPORT obs_property_t *obs_properties_add_float(obs_properties_t *props,
const char *name,
const char *description,
double min, double max,
double step);
EXPORT obs_property_t *obs_properties_add_int_slider(obs_properties_t *props,
const char *name,
const char *description,
int min, int max,
int step);
EXPORT obs_property_t *obs_properties_add_float_slider(obs_properties_t *props,
const char *name,
const char *description,
double min, double max,
double step);
EXPORT obs_property_t *obs_properties_add_text(obs_properties_t *props,
const char *name,
const char *description,
enum obs_text_type type);
EXPORT obs_property_t *
obs_properties_add_path(obs_properties_t *props, const char *name,
const char *description, enum obs_path_type type,
const char *filter, const char *default_path);
EXPORT obs_property_t *obs_properties_add_list(obs_properties_t *props,
const char *name,
const char *description,
enum obs_combo_type type,
enum obs_combo_format format);
EXPORT obs_property_t *obs_properties_add_color(obs_properties_t *props,
const char *name,
const char *description);
EXPORT obs_property_t *
obs_properties_add_button(obs_properties_t *props, const char *name,
const char *text, obs_property_clicked_t callback);
EXPORT obs_property_t *
obs_properties_add_button2(obs_properties_t *props, const char *name,
const char *text, obs_property_clicked_t callback,
void *priv);
EXPORT obs_property_t *obs_properties_add_font(obs_properties_t *props,
const char *name,
const char *description);
EXPORT obs_property_t *
obs_properties_add_editable_list(obs_properties_t *props, const char *name,
const char *description,
enum obs_editable_list_type type,
const char *filter, const char *default_path);
EXPORT obs_property_t *obs_properties_add_frame_rate(obs_properties_t *props,
const char *name,
const char *description);
EXPORT obs_property_t *obs_properties_add_group(obs_properties_t *props,
const char *name,
const char *description,
enum obs_group_type type,
obs_properties_t *group);
typedef bool (*obs_property_modified_t)(obs_properties_t *props,
obs_property_t *property,
obs_data_t *settings);
typedef bool (*obs_property_modified2_t)(void *priv, obs_properties_t *props,
obs_property_t *property,
obs_data_t *settings);
EXPORT void
obs_property_set_modified_callback(obs_property_t *p,
obs_property_modified_t modified);
EXPORT void obs_property_set_modified_callback2(
obs_property_t *p, obs_property_modified2_t modified, void *priv);
EXPORT bool obs_property_modified(obs_property_t *p, obs_data_t *settings);
EXPORT bool obs_property_button_clicked(obs_property_t *p, void *obj);
EXPORT void obs_property_set_visible(obs_property_t *p, bool visible);
EXPORT void obs_property_set_enabled(obs_property_t *p, bool enabled);
EXPORT void obs_property_set_description(obs_property_t *p,
const char *description);
EXPORT void obs_property_set_long_description(obs_property_t *p,
const char *long_description);
EXPORT const char *obs_property_name(obs_property_t *p);
EXPORT const char *obs_property_description(obs_property_t *p);
EXPORT const char *obs_property_long_description(obs_property_t *p);
EXPORT enum obs_property_type obs_property_get_type(obs_property_t *p);
EXPORT bool obs_property_enabled(obs_property_t *p);
EXPORT bool obs_property_visible(obs_property_t *p);
EXPORT bool obs_property_next(obs_property_t **p);
EXPORT int obs_property_int_min(obs_property_t *p);
EXPORT int obs_property_int_max(obs_property_t *p);
EXPORT int obs_property_int_step(obs_property_t *p);
EXPORT enum obs_number_type obs_property_int_type(obs_property_t *p);
EXPORT const char *obs_property_int_suffix(obs_property_t *p);
EXPORT double obs_property_float_min(obs_property_t *p);
EXPORT double obs_property_float_max(obs_property_t *p);
EXPORT double obs_property_float_step(obs_property_t *p);
EXPORT enum obs_number_type obs_property_float_type(obs_property_t *p);
EXPORT const char *obs_property_float_suffix(obs_property_t *p);
EXPORT enum obs_text_type obs_property_text_type(obs_property_t *p);
EXPORT enum obs_text_type obs_property_text_monospace(obs_property_t *p);
EXPORT enum obs_path_type obs_property_path_type(obs_property_t *p);
EXPORT const char *obs_property_path_filter(obs_property_t *p);
EXPORT const char *obs_property_path_default_path(obs_property_t *p);
EXPORT enum obs_combo_type obs_property_list_type(obs_property_t *p);
EXPORT enum obs_combo_format obs_property_list_format(obs_property_t *p);
EXPORT void obs_property_int_set_limits(obs_property_t *p, int min, int max,
int step);
EXPORT void obs_property_float_set_limits(obs_property_t *p, double min,
double max, double step);
EXPORT void obs_property_int_set_suffix(obs_property_t *p, const char *suffix);
EXPORT void obs_property_float_set_suffix(obs_property_t *p,
const char *suffix);
EXPORT void obs_property_text_set_monospace(obs_property_t *p, bool monospace);
EXPORT void obs_property_list_clear(obs_property_t *p);
EXPORT size_t obs_property_list_add_string(obs_property_t *p, const char *name,
const char *val);
EXPORT size_t obs_property_list_add_int(obs_property_t *p, const char *name,
long long val);
EXPORT size_t obs_property_list_add_float(obs_property_t *p, const char *name,
double val);
EXPORT void obs_property_list_insert_string(obs_property_t *p, size_t idx,
const char *name, const char *val);
EXPORT void obs_property_list_insert_int(obs_property_t *p, size_t idx,
const char *name, long long val);
EXPORT void obs_property_list_insert_float(obs_property_t *p, size_t idx,
const char *name, double val);
EXPORT void obs_property_list_item_disable(obs_property_t *p, size_t idx,
bool disabled);
EXPORT bool obs_property_list_item_disabled(obs_property_t *p, size_t idx);
EXPORT void obs_property_list_item_remove(obs_property_t *p, size_t idx);
EXPORT size_t obs_property_list_item_count(obs_property_t *p);
EXPORT const char *obs_property_list_item_name(obs_property_t *p, size_t idx);
EXPORT const char *obs_property_list_item_string(obs_property_t *p, size_t idx);
EXPORT long long obs_property_list_item_int(obs_property_t *p, size_t idx);
EXPORT double obs_property_list_item_float(obs_property_t *p, size_t idx);
EXPORT enum obs_editable_list_type
obs_property_editable_list_type(obs_property_t *p);
EXPORT const char *obs_property_editable_list_filter(obs_property_t *p);
EXPORT const char *obs_property_editable_list_default_path(obs_property_t *p);
EXPORT void obs_property_frame_rate_clear(obs_property_t *p);
EXPORT void obs_property_frame_rate_options_clear(obs_property_t *p);
EXPORT void obs_property_frame_rate_fps_ranges_clear(obs_property_t *p);
EXPORT size_t obs_property_frame_rate_option_add(obs_property_t *p,
const char *name,
const char *description);
EXPORT size_t obs_property_frame_rate_fps_range_add(
obs_property_t *p, struct media_frames_per_second min,
struct media_frames_per_second max);
EXPORT void obs_property_frame_rate_option_insert(obs_property_t *p, size_t idx,
const char *name,
const char *description);
EXPORT void
obs_property_frame_rate_fps_range_insert(obs_property_t *p, size_t idx,
struct media_frames_per_second min,
struct media_frames_per_second max);
EXPORT size_t obs_property_frame_rate_options_count(obs_property_t *p);
EXPORT const char *obs_property_frame_rate_option_name(obs_property_t *p,
size_t idx);
EXPORT const char *obs_property_frame_rate_option_description(obs_property_t *p,
size_t idx);
EXPORT size_t obs_property_frame_rate_fps_ranges_count(obs_property_t *p);
EXPORT struct media_frames_per_second
obs_property_frame_rate_fps_range_min(obs_property_t *p, size_t idx);
EXPORT struct media_frames_per_second
obs_property_frame_rate_fps_range_max(obs_property_t *p, size_t idx);
EXPORT enum obs_group_type obs_property_group_type(obs_property_t *p);
EXPORT obs_properties_t *obs_property_group_content(obs_property_t *p);
#if !defined(SWIG)
OBS_DEPRECATED
EXPORT enum obs_text_type obs_proprety_text_type(obs_property_t *p);
#endif
#if defined(__cplusplus)
}
#endif
