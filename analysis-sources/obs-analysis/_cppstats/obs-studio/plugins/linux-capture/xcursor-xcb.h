#pragma once
#include <obs.h>
#include <xcb/xfixes.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct {
unsigned int last_serial;
unsigned int last_width;
unsigned int last_height;
gs_texture_t *tex;
int x;
int y;
int x_org;
int y_org;
float x_render;
float y_render;
} xcb_xcursor_t;
xcb_xcursor_t *xcb_xcursor_init(xcb_connection_t *xcb);
void xcb_xcursor_destroy(xcb_xcursor_t *data);
void xcb_xcursor_update(xcb_xcursor_t *data,
xcb_xfixes_get_cursor_image_reply_t *xc);
void xcb_xcursor_render(xcb_xcursor_t *data);
void xcb_xcursor_offset(xcb_xcursor_t *data, const int x_org, const int y_org);
#if defined(__cplusplus)
}
#endif
