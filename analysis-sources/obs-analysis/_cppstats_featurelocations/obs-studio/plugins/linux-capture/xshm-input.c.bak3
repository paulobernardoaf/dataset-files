
















#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <xcb/randr.h>
#include <xcb/shm.h>
#include <xcb/xfixes.h>
#include <xcb/xinerama.h>

#include <obs-module.h>
#include <util/dstr.h>
#include "xcursor-xcb.h"
#include "xhelpers.h"

#define XSHM_DATA(voidptr) struct xshm_data *data = voidptr;

#define blog(level, msg, ...) blog(level, "xshm-input: " msg, ##__VA_ARGS__)

struct xshm_data {
obs_source_t *source;

xcb_connection_t *xcb;
xcb_screen_t *xcb_screen;
xcb_shm_t *xshm;
xcb_xcursor_t *cursor;

char *server;
uint_fast32_t screen_id;
int_fast32_t x_org;
int_fast32_t y_org;
int_fast32_t width;
int_fast32_t height;

gs_texture_t *texture;

bool show_cursor;
bool use_xinerama;
bool use_randr;
bool advanced;
};








static inline void xshm_resize_texture(struct xshm_data *data)
{
if (data->texture)
gs_texture_destroy(data->texture);
data->texture = gs_texture_create(data->width, data->height, GS_BGRA, 1,
NULL, GS_DYNAMIC);
}




static bool xshm_check_extensions(xcb_connection_t *xcb)
{
bool ok = true;

if (!xcb_get_extension_data(xcb, &xcb_shm_id)->present) {
blog(LOG_ERROR, "Missing SHM extension !");
ok = false;
}

if (!xcb_get_extension_data(xcb, &xcb_xinerama_id)->present)
blog(LOG_INFO, "Missing Xinerama extension !");

if (!xcb_get_extension_data(xcb, &xcb_randr_id)->present)
blog(LOG_INFO, "Missing Randr extension !");

return ok;
}






static int_fast32_t xshm_update_geometry(struct xshm_data *data)
{
int_fast32_t old_width = data->width;
int_fast32_t old_height = data->height;

if (data->use_randr) {
if (randr_screen_geo(data->xcb, data->screen_id, &data->x_org,
&data->y_org, &data->width, &data->height,
&data->xcb_screen, NULL) < 0) {
return -1;
}
} else if (data->use_xinerama) {
if (xinerama_screen_geo(data->xcb, data->screen_id,
&data->x_org, &data->y_org,
&data->width, &data->height) < 0) {
return -1;
}
data->xcb_screen = xcb_get_screen(data->xcb, 0);
} else {
data->x_org = 0;
data->y_org = 0;
if (x11_screen_geo(data->xcb, data->screen_id, &data->width,
&data->height) < 0) {
return -1;
}
data->xcb_screen = xcb_get_screen(data->xcb, data->screen_id);
}

if (!data->width || !data->height) {
blog(LOG_ERROR, "Failed to get geometry");
return -1;
}

blog(LOG_INFO,
"Geometry %" PRIdFAST32 "x%" PRIdFAST32 " @ %" PRIdFAST32
",%" PRIdFAST32,
data->width, data->height, data->x_org, data->y_org);

if (old_width == data->width && old_height == data->height)
return 0;

return 1;
}




static const char *xshm_getname(void *unused)
{
UNUSED_PARAMETER(unused);
return obs_module_text("X11SharedMemoryScreenInput");
}




static void xshm_capture_stop(struct xshm_data *data)
{
obs_enter_graphics();

if (data->texture) {
gs_texture_destroy(data->texture);
data->texture = NULL;
}
if (data->cursor) {
xcb_xcursor_destroy(data->cursor);
data->cursor = NULL;
}

obs_leave_graphics();

if (data->xshm) {
xshm_xcb_detach(data->xshm);
data->xshm = NULL;
}

if (data->xcb) {
xcb_disconnect(data->xcb);
data->xcb = NULL;
}

if (data->server) {
bfree(data->server);
data->server = NULL;
}
}




static void xshm_capture_start(struct xshm_data *data)
{
const char *server = (data->advanced && *data->server) ? data->server
: NULL;

data->xcb = xcb_connect(server, NULL);
if (!data->xcb || xcb_connection_has_error(data->xcb)) {
blog(LOG_ERROR, "Unable to open X display !");
goto fail;
}

if (!xshm_check_extensions(data->xcb))
goto fail;

data->use_randr = randr_is_active(data->xcb) ? true : false;
data->use_xinerama = xinerama_is_active(data->xcb) ? true : false;

if (xshm_update_geometry(data) < 0) {
blog(LOG_ERROR, "failed to update geometry !");
goto fail;
}

data->xshm = xshm_xcb_attach(data->xcb, data->width, data->height);
if (!data->xshm) {
blog(LOG_ERROR, "failed to attach shm !");
goto fail;
}

data->cursor = xcb_xcursor_init(data->xcb);
xcb_xcursor_offset(data->cursor, data->x_org, data->y_org);

obs_enter_graphics();

xshm_resize_texture(data);

obs_leave_graphics();

return;
fail:
xshm_capture_stop(data);
}




static void xshm_update(void *vptr, obs_data_t *settings)
{
XSHM_DATA(vptr);

xshm_capture_stop(data);

data->screen_id = obs_data_get_int(settings, "screen");
data->show_cursor = obs_data_get_bool(settings, "show_cursor");
data->advanced = obs_data_get_bool(settings, "advanced");
data->server = bstrdup(obs_data_get_string(settings, "server"));

xshm_capture_start(data);
}




static void xshm_defaults(obs_data_t *defaults)
{
obs_data_set_default_int(defaults, "screen", 0);
obs_data_set_default_bool(defaults, "show_cursor", true);
obs_data_set_default_bool(defaults, "advanced", false);
}




static bool xshm_toggle_advanced(obs_properties_t *props, obs_property_t *p,
obs_data_t *settings)
{
UNUSED_PARAMETER(p);
const bool visible = obs_data_get_bool(settings, "advanced");
obs_property_t *server = obs_properties_get(props, "server");

obs_property_set_visible(server, visible);


obs_property_modified(server, settings);

return true;
}




static bool xshm_server_changed(obs_properties_t *props, obs_property_t *p,
obs_data_t *settings)
{
UNUSED_PARAMETER(p);

bool advanced = obs_data_get_bool(settings, "advanced");
int_fast32_t old_screen = obs_data_get_int(settings, "screen");
const char *server = obs_data_get_string(settings, "server");
obs_property_t *screens = obs_properties_get(props, "screen");


server = (advanced && *server) ? server : NULL;

obs_property_list_clear(screens);

xcb_connection_t *xcb = xcb_connect(server, NULL);
if (!xcb || xcb_connection_has_error(xcb)) {
obs_property_set_enabled(screens, false);
return true;
}

struct dstr screen_info;
dstr_init(&screen_info);
bool randr = randr_is_active(xcb);
bool xinerama = xinerama_is_active(xcb);
int_fast32_t count =
(randr) ? randr_screen_count(xcb)
: (xinerama)
? xinerama_screen_count(xcb)
: xcb_setup_roots_length(xcb_get_setup(xcb));

for (int_fast32_t i = 0; i < count; ++i) {
char *name;
char name_tmp[12];
int_fast32_t x, y, w, h;
x = y = w = h = 0;

name = NULL;
if (randr)
randr_screen_geo(xcb, i, &x, &y, &w, &h, NULL, &name);
else if (xinerama)
xinerama_screen_geo(xcb, i, &x, &y, &w, &h);
else
x11_screen_geo(xcb, i, &w, &h);

if (name == NULL) {
sprintf(name_tmp, "%" PRIuFAST32, i);
name = name_tmp;
}

dstr_printf(&screen_info,
"Screen %s (%" PRIuFAST32 "x%" PRIuFAST32
" @ %" PRIuFAST32 ",%" PRIuFAST32 ")",
name, w, h, x, y);

if (name != name_tmp)
free(name);

if (h > 0 && w > 0)
obs_property_list_add_int(screens, screen_info.array,
i);
}


if (old_screen + 1 > count) {
dstr_printf(&screen_info, "Screen %" PRIuFAST32 " (not found)",
old_screen);
size_t index = obs_property_list_add_int(
screens, screen_info.array, old_screen);
obs_property_list_item_disable(screens, index, true);
}

dstr_free(&screen_info);

xcb_disconnect(xcb);
obs_property_set_enabled(screens, true);

return true;
}




static obs_properties_t *xshm_properties(void *vptr)
{
XSHM_DATA(vptr);

obs_properties_t *props = obs_properties_create();

obs_properties_add_list(props, "screen", obs_module_text("Screen"),
OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
obs_properties_add_bool(props, "show_cursor",
obs_module_text("CaptureCursor"));
obs_property_t *advanced = obs_properties_add_bool(
props, "advanced", obs_module_text("AdvancedSettings"));
obs_property_t *server = obs_properties_add_text(
props, "server", obs_module_text("XServer"), OBS_TEXT_DEFAULT);

obs_property_set_modified_callback(advanced, xshm_toggle_advanced);
obs_property_set_modified_callback(server, xshm_server_changed);


obs_data_t *settings = obs_source_get_settings(data->source);
obs_property_modified(server, settings);
obs_data_release(settings);

return props;
}




static void xshm_destroy(void *vptr)
{
XSHM_DATA(vptr);

if (!data)
return;

xshm_capture_stop(data);

bfree(data);
}




static void *xshm_create(obs_data_t *settings, obs_source_t *source)
{
struct xshm_data *data = bzalloc(sizeof(struct xshm_data));
data->source = source;

xshm_update(data, settings);

return data;
}




static void xshm_video_tick(void *vptr, float seconds)
{
UNUSED_PARAMETER(seconds);
XSHM_DATA(vptr);

if (!data->texture)
return;
if (!obs_source_showing(data->source))
return;

xcb_shm_get_image_cookie_t img_c;
xcb_shm_get_image_reply_t *img_r;
xcb_xfixes_get_cursor_image_cookie_t cur_c;
xcb_xfixes_get_cursor_image_reply_t *cur_r;

img_c = xcb_shm_get_image_unchecked(data->xcb, data->xcb_screen->root,
data->x_org, data->y_org,
data->width, data->height, ~0,
XCB_IMAGE_FORMAT_Z_PIXMAP,
data->xshm->seg, 0);
cur_c = xcb_xfixes_get_cursor_image_unchecked(data->xcb);

img_r = xcb_shm_get_image_reply(data->xcb, img_c, NULL);
cur_r = xcb_xfixes_get_cursor_image_reply(data->xcb, cur_c, NULL);

if (!img_r)
goto exit;

obs_enter_graphics();

gs_texture_set_image(data->texture, (void *)data->xshm->data,
data->width * 4, false);
xcb_xcursor_update(data->cursor, cur_r);

obs_leave_graphics();

exit:
free(img_r);
free(cur_r);
}




static void xshm_video_render(void *vptr, gs_effect_t *effect)
{
XSHM_DATA(vptr);

effect = obs_get_base_effect(OBS_EFFECT_OPAQUE);

if (!data->texture)
return;

gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");
gs_effect_set_texture(image, data->texture);

while (gs_effect_loop(effect, "Draw")) {
gs_draw_sprite(data->texture, 0, 0, 0);
}

if (data->show_cursor) {
effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

while (gs_effect_loop(effect, "Draw")) {
xcb_xcursor_render(data->cursor);
}
}
}




static uint32_t xshm_getwidth(void *vptr)
{
XSHM_DATA(vptr);
return data->width;
}




static uint32_t xshm_getheight(void *vptr)
{
XSHM_DATA(vptr);
return data->height;
}

struct obs_source_info xshm_input = {
.id = "xshm_input",
.type = OBS_SOURCE_TYPE_INPUT,
.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
OBS_SOURCE_DO_NOT_DUPLICATE,
.get_name = xshm_getname,
.create = xshm_create,
.destroy = xshm_destroy,
.update = xshm_update,
.get_defaults = xshm_defaults,
.get_properties = xshm_properties,
.video_tick = xshm_video_tick,
.video_render = xshm_video_render,
.get_width = xshm_getwidth,
.get_height = xshm_getheight,
.icon_type = OBS_ICON_TYPE_DESKTOP_CAPTURE,
};
