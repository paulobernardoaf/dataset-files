#pragma once
#include <obs.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct {
Display *dpy;
float render_x;
float render_y;
unsigned long last_serial;
uint_fast32_t last_width;
uint_fast32_t last_height;
gs_texture_t *tex;
int_fast32_t x, y;
int_fast32_t x_org;
int_fast32_t y_org;
} xcursor_t;
xcursor_t *xcursor_init(Display *dpy);
void xcursor_destroy(xcursor_t *data);
void xcursor_tick(xcursor_t *data);
void xcursor_render(xcursor_t *data, int x_offset, int y_offset);
void xcursor_offset(xcursor_t *data, int_fast32_t x_org, int_fast32_t y_org);
#if defined(__cplusplus)
}
#endif
