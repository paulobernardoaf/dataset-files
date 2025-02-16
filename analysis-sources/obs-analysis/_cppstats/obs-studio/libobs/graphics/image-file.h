#pragma once
#include "graphics.h"
#include "libnsgif/libnsgif.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct gs_image_file {
gs_texture_t *texture;
enum gs_color_format format;
uint32_t cx;
uint32_t cy;
bool is_animated_gif;
bool frame_updated;
bool loaded;
gif_animation gif;
uint8_t *gif_data;
uint8_t **animation_frame_cache;
uint8_t *animation_frame_data;
uint64_t cur_time;
int cur_frame;
int cur_loop;
int last_decoded_frame;
uint8_t *texture_data;
gif_bitmap_callback_vt bitmap_callbacks;
};
struct gs_image_file2 {
struct gs_image_file image;
uint64_t mem_usage;
};
typedef struct gs_image_file gs_image_file_t;
typedef struct gs_image_file2 gs_image_file2_t;
EXPORT void gs_image_file_init(gs_image_file_t *image, const char *file);
EXPORT void gs_image_file_free(gs_image_file_t *image);
EXPORT void gs_image_file_init_texture(gs_image_file_t *image);
EXPORT bool gs_image_file_tick(gs_image_file_t *image,
uint64_t elapsed_time_ns);
EXPORT void gs_image_file_update_texture(gs_image_file_t *image);
EXPORT void gs_image_file2_init(gs_image_file2_t *if2, const char *file);
static void gs_image_file2_free(gs_image_file2_t *if2)
{
gs_image_file_free(&if2->image);
if2->mem_usage = 0;
}
static inline void gs_image_file2_init_texture(gs_image_file2_t *if2)
{
gs_image_file_init_texture(&if2->image);
}
static inline bool gs_image_file2_tick(gs_image_file2_t *if2,
uint64_t elapsed_time_ns)
{
return gs_image_file_tick(&if2->image, elapsed_time_ns);
}
static inline void gs_image_file2_update_texture(gs_image_file2_t *if2)
{
gs_image_file_update_texture(&if2->image);
}
#if defined(__cplusplus)
}
#endif
