
















#pragma once

#include "obs.h"
#include "obs-internal.h"
#include "graphics/matrix4.h"



struct item_action {
bool visible;
uint64_t timestamp;
};

struct obs_scene_item {
volatile long ref;
volatile bool removed;

bool is_group;
bool update_transform;
bool update_group_resize;

int64_t id;

struct obs_scene *parent;
struct obs_source *source;
volatile long active_refs;
volatile long defer_update;
volatile long defer_group_resize;
bool user_visible;
bool visible;
bool selected;
bool locked;

gs_texrender_t *item_render;
struct obs_sceneitem_crop crop;

struct vec2 pos;
struct vec2 scale;
float rot;
uint32_t align;



uint32_t last_width;
uint32_t last_height;

struct vec2 output_scale;
enum obs_scale_type scale_filter;

struct matrix4 box_transform;
struct vec2 box_scale;
struct matrix4 draw_transform;

enum obs_bounds_type bounds_type;
uint32_t bounds_align;
struct vec2 bounds;

obs_hotkey_pair_id toggle_visibility;

obs_data_t *private_settings;

pthread_mutex_t actions_mutex;
DARRAY(struct item_action) audio_actions;


struct obs_scene_item *prev;
struct obs_scene_item *next;
};

struct obs_scene {
struct obs_source *source;

bool is_group;
bool custom_size;
uint32_t cx;
uint32_t cy;

int64_t id_counter;

pthread_mutex_t video_mutex;
pthread_mutex_t audio_mutex;
struct obs_scene_item *first_item;
};
