#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include "pictures.h"
#include "events.h"
struct vout_display_sys_t
{
xcb_connection_t *conn;
xcb_window_t window; 
xcb_gcontext_t gc; 
xcb_shm_seg_t segment; 
bool attached;
uint8_t depth; 
video_format_t fmt;
};
static void Prepare(vout_display_t *vd, picture_t *pic, subpicture_t *subpic,
vlc_tick_t date)
{
vout_display_sys_t *sys = vd->sys;
const picture_buffer_t *buf = pic->p_sys;
xcb_connection_t *conn = sys->conn;
sys->attached = false;
if (sys->segment == 0)
return; 
if (buf->fd == -1)
return; 
int fd = vlc_dup(buf->fd);
if (fd == -1)
return;
xcb_void_cookie_t c = xcb_shm_attach_fd_checked(conn, sys->segment, fd, 1);
xcb_generic_error_t *e = xcb_request_check(conn, c);
if (e != NULL) 
{
free(e);
return;
}
sys->attached = true;
(void) subpic; (void) date;
}
static void Display (vout_display_t *vd, picture_t *pic)
{
vout_display_sys_t *sys = vd->sys;
xcb_connection_t *conn = sys->conn;
const picture_buffer_t *buf = pic->p_sys;
xcb_shm_seg_t segment = sys->segment;
xcb_void_cookie_t ck;
vlc_xcb_Manage(vd, sys->conn);
if (sys->attached)
ck = xcb_shm_put_image_checked(conn, sys->window, sys->gc,
pic->p->i_pitch / pic->p->i_pixel_pitch,
pic->p->i_lines,
sys->fmt.i_x_offset,
sys->fmt.i_y_offset,
sys->fmt.i_visible_width,
sys->fmt.i_visible_height,
0, 0, sys->depth, XCB_IMAGE_FORMAT_Z_PIXMAP,
0, segment, buf->offset);
else {
const size_t offset = sys->fmt.i_y_offset * pic->p->i_pitch;
const unsigned lines = pic->p->i_lines - sys->fmt.i_y_offset;
ck = xcb_put_image_checked(conn, XCB_IMAGE_FORMAT_Z_PIXMAP,
sys->window, sys->gc,
pic->p->i_pitch / pic->p->i_pixel_pitch,
lines, -sys->fmt.i_x_offset, 0, 0, sys->depth,
pic->p->i_pitch * lines,
pic->p->p_pixels + offset);
}
xcb_generic_error_t *e = xcb_request_check(conn, ck);
if (e != NULL) {
msg_Err(vd, "%s: X11 error %d", "cannot put image", e->error_code);
free(e);
}
if (sys->attached)
xcb_shm_detach(conn, segment);
}
static int Control(vout_display_t *vd, int query, va_list ap)
{
vout_display_sys_t *sys = vd->sys;
switch (query) {
case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE:
case VOUT_DISPLAY_CHANGE_ZOOM:
case VOUT_DISPLAY_CHANGE_DISPLAY_FILLED:
case VOUT_DISPLAY_CHANGE_SOURCE_ASPECT:
case VOUT_DISPLAY_CHANGE_SOURCE_CROP:
{
const vout_display_cfg_t *cfg = va_arg(ap, const vout_display_cfg_t *);
video_format_t src, *fmt = &sys->fmt;
vout_display_place_t place;
int ret = VLC_SUCCESS;
vout_display_PlacePicture(&place, &vd->source, cfg);
uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
const uint32_t values[] = {
place.x, place.y, place.width, place.height
};
if (place.width != sys->fmt.i_visible_width
|| place.height != sys->fmt.i_visible_height)
{
mask |= XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
ret = VLC_EGENERIC;
}
xcb_configure_window(sys->conn, sys->window, mask, values);
video_format_ApplyRotation(&src, &vd->source);
fmt->i_width = src.i_width * place.width / src.i_visible_width;
fmt->i_height = src.i_height * place.height / src.i_visible_height;
fmt->i_visible_width = place.width;
fmt->i_visible_height = place.height;
fmt->i_x_offset = src.i_x_offset * place.width / src.i_visible_width;
fmt->i_y_offset = src.i_y_offset * place.height / src.i_visible_height;
return ret;
}
case VOUT_DISPLAY_RESET_PICTURES:
{
va_arg(ap, const vout_display_cfg_t *);
*va_arg(ap, video_format_t *) = sys->fmt;
return VLC_SUCCESS;
}
default:
msg_Err (vd, "Unknown request in XCB vout display");
return VLC_EGENERIC;
}
}
static void Close(vout_display_t *vd)
{
vout_display_sys_t *sys = vd->sys;
xcb_disconnect(sys->conn);
free(sys);
}
static xcb_visualid_t DepthToFormat(const xcb_setup_t *setup,
const xcb_depth_t *depth,
video_format_t *restrict f)
{
const xcb_visualtype_t *vt = xcb_depth_visuals(depth);
for (int i = xcb_depth_visuals_length(depth); i > 0; i--, vt++)
if (vlc_xcb_VisualToFormat(setup, depth->depth, vt, f))
return vt->visual_id;
return 0;
}
static xcb_visualid_t ScreenToFormat(const xcb_setup_t *setup,
const xcb_screen_t *screen,
uint8_t *restrict bits,
video_format_t *restrict fmtp)
{
xcb_visualid_t visual = 0;
*bits = 0;
for (xcb_depth_iterator_t it = xcb_screen_allowed_depths_iterator(screen);
it.rem > 0;
xcb_depth_next(&it))
{
const xcb_depth_t *depth = it.data;
video_format_t fmt;
xcb_visualid_t vid;
if (depth->depth <= *bits)
continue; 
video_format_ApplyRotation(&fmt, fmtp);
vid = DepthToFormat(setup, depth, &fmt);
if (vid != 0)
{
*bits = depth->depth;
*fmtp = fmt;
visual = vid;
}
}
return visual;
}
static int Open (vout_display_t *vd, const vout_display_cfg_t *cfg,
video_format_t *fmtp, vlc_video_context *context)
{
vout_display_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
vd->sys = sys;
xcb_connection_t *conn;
const xcb_screen_t *scr;
if (vlc_xcb_parent_Create(vd, cfg->window, &conn, &scr) != VLC_SUCCESS)
{
free (sys);
return VLC_EGENERIC;
}
sys->conn = conn;
const xcb_setup_t *setup = xcb_get_setup (conn);
xcb_visualid_t vid = ScreenToFormat(setup, scr, &sys->depth, fmtp);
if (vid == 0) {
msg_Err(vd, "no supported visual & pixel format");
goto error;
}
msg_Dbg(vd, "using X11 visual ID 0x%"PRIx32, vid);
msg_Dbg(vd, " %"PRIu8" bits depth", sys->depth);
xcb_colormap_t cmap;
if (vid != scr->root_visual)
{
cmap = xcb_generate_id (conn);
xcb_create_colormap (conn, XCB_COLORMAP_ALLOC_NONE,
cmap, scr->root, vid);
}
else
cmap = scr->default_colormap;
const uint32_t mask =
XCB_CW_BACK_PIXEL |
XCB_CW_BORDER_PIXEL |
XCB_CW_EVENT_MASK |
XCB_CW_COLORMAP;
const uint32_t values[] = {
scr->black_pixel,
scr->black_pixel,
0,
cmap,
};
vout_display_place_t place;
vout_display_PlacePicture(&place, &vd->source, cfg);
sys->window = xcb_generate_id (conn);
sys->gc = xcb_generate_id (conn);
xcb_create_window(conn, sys->depth, sys->window, cfg->window->handle.xid,
place.x, place.y, place.width, place.height, 0,
XCB_WINDOW_CLASS_INPUT_OUTPUT, vid, mask, values);
xcb_map_window(conn, sys->window);
xcb_create_gc(conn, sys->gc, sys->window, 0, NULL);
msg_Dbg (vd, "using X11 window %08"PRIx32, sys->window);
msg_Dbg (vd, "using X11 graphic context %08"PRIx32, sys->gc);
if (XCB_shm_Check (VLC_OBJECT(vd), conn))
sys->segment = xcb_generate_id(conn);
else
sys->segment = 0;
sys->fmt = *fmtp;
vd->prepare = Prepare;
vd->display = Display;
vd->control = Control;
vd->close = Close;
(void) context;
return VLC_SUCCESS;
error:
Close (vd);
return VLC_EGENERIC;
}
vlc_module_begin()
set_shortname(N_("X11"))
set_description(N_("X11 video output (XCB)"))
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VOUT)
set_callback_display(Open, 100)
add_shortcut("xcb-x11", "x11")
add_obsolete_bool("x11-shm") 
vlc_module_end()
