#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <stdio.h>
#include "gl-subsystem.h"
#include <glad/glad_glx.h>
static const int ctx_attribs[] = {
#if defined(_DEBUG)
GLX_CONTEXT_FLAGS_ARB,
GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
GLX_CONTEXT_PROFILE_MASK_ARB,
GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
GLX_CONTEXT_MAJOR_VERSION_ARB,
3,
GLX_CONTEXT_MINOR_VERSION_ARB,
3,
None,
};
static int ctx_pbuffer_attribs[] = {GLX_PBUFFER_WIDTH, 2, GLX_PBUFFER_HEIGHT, 2,
None};
static int ctx_visual_attribs[] = {GLX_STENCIL_SIZE,
0,
GLX_DEPTH_SIZE,
0,
GLX_BUFFER_SIZE,
32,
GLX_ALPHA_SIZE,
8,
GLX_DOUBLEBUFFER,
true,
GLX_X_RENDERABLE,
true,
None};
struct gl_windowinfo {
GLXFBConfig config;
xcb_window_t window;
int screen;
};
struct gl_platform {
Display *display;
GLXContext context;
GLXPbuffer pbuffer;
};
static int get_screen_num_from_xcb_screen(xcb_connection_t *xcb_conn,
xcb_screen_t *screen)
{
xcb_screen_iterator_t iter =
xcb_setup_roots_iterator(xcb_get_setup(xcb_conn));
int screen_num = 0;
for (; iter.rem; xcb_screen_next(&iter), ++screen_num)
if (iter.data == screen)
return screen_num;
return -1;
}
static xcb_screen_t *get_screen_from_root(xcb_connection_t *xcb_conn,
xcb_window_t root)
{
xcb_screen_iterator_t iter =
xcb_setup_roots_iterator(xcb_get_setup(xcb_conn));
while (iter.rem) {
if (iter.data->root == root)
return iter.data;
xcb_screen_next(&iter);
}
return 0;
}
static inline int get_screen_num_from_root(xcb_connection_t *xcb_conn,
xcb_window_t root)
{
xcb_screen_t *screen = get_screen_from_root(xcb_conn, root);
if (!screen)
return -1;
return get_screen_num_from_xcb_screen(xcb_conn, screen);
}
static xcb_get_geometry_reply_t *get_window_geometry(xcb_connection_t *xcb_conn,
xcb_drawable_t drawable)
{
xcb_get_geometry_cookie_t cookie;
xcb_generic_error_t *error;
xcb_get_geometry_reply_t *reply;
cookie = xcb_get_geometry(xcb_conn, drawable);
reply = xcb_get_geometry_reply(xcb_conn, cookie, &error);
if (error) {
blog(LOG_ERROR, "Failed to fetch parent window geometry!");
free(error);
free(reply);
return 0;
}
free(error);
return reply;
}
static bool gl_context_create(struct gl_platform *plat)
{
Display *display = plat->display;
int frame_buf_config_count = 0;
GLXFBConfig *config = NULL;
GLXContext context;
bool success = false;
if (!GLAD_GLX_ARB_create_context) {
blog(LOG_ERROR, "ARB_GLX_create_context not supported!");
return false;
}
config = glXChooseFBConfig(display, DefaultScreen(display),
ctx_visual_attribs, &frame_buf_config_count);
if (!config) {
blog(LOG_ERROR, "Failed to create OpenGL frame buffer config");
return false;
}
context = glXCreateContextAttribsARB(display, config[0], NULL, true,
ctx_attribs);
if (!context) {
blog(LOG_ERROR, "Failed to create OpenGL context.");
goto error;
}
plat->context = context;
plat->display = display;
plat->pbuffer =
glXCreatePbuffer(display, config[0], ctx_pbuffer_attribs);
if (!plat->pbuffer) {
blog(LOG_ERROR, "Failed to create OpenGL pbuffer");
goto error;
}
success = true;
error:
XFree(config);
XSync(display, false);
return success;
}
static void gl_context_destroy(struct gl_platform *plat)
{
Display *display = plat->display;
glXMakeContextCurrent(display, None, None, NULL);
glXDestroyContext(display, plat->context);
bfree(plat);
}
extern struct gl_windowinfo *
gl_windowinfo_create(const struct gs_init_data *info)
{
UNUSED_PARAMETER(info);
return bmalloc(sizeof(struct gl_windowinfo));
}
extern void gl_windowinfo_destroy(struct gl_windowinfo *info)
{
bfree(info);
}
static Display *open_windowless_display(void)
{
Display *display = XOpenDisplay(NULL);
xcb_connection_t *xcb_conn;
xcb_screen_iterator_t screen_iterator;
xcb_screen_t *screen;
int screen_num;
if (!display) {
blog(LOG_ERROR, "Unable to open new X connection!");
return NULL;
}
xcb_conn = XGetXCBConnection(display);
if (!xcb_conn) {
blog(LOG_ERROR, "Unable to get XCB connection to main display");
goto error;
}
screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(xcb_conn));
screen = screen_iterator.data;
if (!screen) {
blog(LOG_ERROR, "Unable to get screen root");
goto error;
}
screen_num = get_screen_num_from_root(xcb_conn, screen->root);
if (screen_num == -1) {
blog(LOG_ERROR, "Unable to get screen number from root");
goto error;
}
if (!gladLoadGLX(display, screen_num)) {
blog(LOG_ERROR, "Unable to load GLX entry functions.");
goto error;
}
return display;
error:
if (display)
XCloseDisplay(display);
return NULL;
}
static int x_error_handler(Display *display, XErrorEvent *error)
{
char str1[512];
char str2[512];
char str3[512];
XGetErrorText(display, error->error_code, str1, sizeof(str1));
XGetErrorText(display, error->request_code, str2, sizeof(str2));
XGetErrorText(display, error->minor_code, str3, sizeof(str3));
blog(LOG_ERROR,
"X Error: %s, Major opcode: %s, "
"Minor opcode: %s, Serial: %lu",
str1, str2, str3, error->serial);
return 0;
}
extern struct gl_platform *gl_platform_create(gs_device_t *device,
uint32_t adapter)
{
struct gl_platform *plat = bmalloc(sizeof(struct gl_platform));
Display *display = open_windowless_display();
if (!display) {
goto fail_display_open;
}
XSetEventQueueOwner(display, XCBOwnsEventQueue);
XSetErrorHandler(x_error_handler);
device->plat = plat;
plat->display = display;
if (!gl_context_create(plat)) {
blog(LOG_ERROR, "Failed to create context!");
goto fail_context_create;
}
if (!glXMakeContextCurrent(plat->display, plat->pbuffer, plat->pbuffer,
plat->context)) {
blog(LOG_ERROR, "Failed to make context current.");
goto fail_make_current;
}
if (!gladLoadGL()) {
blog(LOG_ERROR, "Failed to load OpenGL entry functions.");
goto fail_load_gl;
}
goto success;
fail_make_current:
gl_context_destroy(plat);
fail_context_create:
fail_load_gl:
XCloseDisplay(display);
fail_display_open:
bfree(plat);
plat = NULL;
success:
UNUSED_PARAMETER(adapter);
return plat;
}
extern void gl_platform_destroy(struct gl_platform *plat)
{
if (!plat) 
return;
gl_context_destroy(plat);
}
extern bool gl_platform_init_swapchain(struct gs_swap_chain *swap)
{
Display *display = swap->device->plat->display;
xcb_connection_t *xcb_conn = XGetXCBConnection(display);
xcb_window_t wid = xcb_generate_id(xcb_conn);
xcb_window_t parent = swap->info.window.id;
xcb_get_geometry_reply_t *geometry =
get_window_geometry(xcb_conn, parent);
bool status = false;
int screen_num;
int visual;
GLXFBConfig *fb_config;
if (!geometry)
goto fail_geometry_request;
screen_num = get_screen_num_from_root(xcb_conn, geometry->root);
if (screen_num == -1) {
goto fail_screen;
}
{
int num_configs;
fb_config = glXChooseFBConfig(display, screen_num,
ctx_visual_attribs, &num_configs);
if (!fb_config || !num_configs) {
blog(LOG_ERROR, "Failed to find FBConfig!");
goto fail_fb_config;
}
}
{
int error = glXGetFBConfigAttrib(display, fb_config[0],
GLX_VISUAL_ID, &visual);
if (error) {
blog(LOG_ERROR, "Bad call to GetFBConfigAttrib!");
goto fail_visual_id;
}
}
xcb_colormap_t colormap = xcb_generate_id(xcb_conn);
uint32_t mask = XCB_CW_BORDER_PIXEL | XCB_CW_COLORMAP;
uint32_t mask_values[] = {0, colormap, 0};
xcb_create_colormap(xcb_conn, XCB_COLORMAP_ALLOC_NONE, colormap, parent,
visual);
xcb_create_window(xcb_conn, 24 , wid, parent, 0, 0,
geometry->width, geometry->height, 0, 0, visual, mask,
mask_values);
swap->wi->config = fb_config[0];
swap->wi->window = wid;
xcb_map_window(xcb_conn, wid);
XFree(fb_config);
status = true;
goto success;
fail_visual_id:
XFree(fb_config);
fail_fb_config:
fail_screen:
fail_geometry_request:
success:
free(geometry);
return status;
}
extern void gl_platform_cleanup_swapchain(struct gs_swap_chain *swap)
{
UNUSED_PARAMETER(swap);
}
extern void device_enter_context(gs_device_t *device)
{
GLXContext context = device->plat->context;
Display *display = device->plat->display;
if (device->cur_swap) {
XID window = device->cur_swap->wi->window;
if (!glXMakeContextCurrent(display, window, window, context)) {
blog(LOG_ERROR, "Failed to make context current.");
}
} else {
GLXPbuffer pbuf = device->plat->pbuffer;
if (!glXMakeContextCurrent(display, pbuf, pbuf, context)) {
blog(LOG_ERROR, "Failed to make context current.");
}
}
}
extern void device_leave_context(gs_device_t *device)
{
Display *display = device->plat->display;
if (!glXMakeContextCurrent(display, None, None, NULL)) {
blog(LOG_ERROR, "Failed to reset current context.");
}
}
void *device_get_device_obj(gs_device_t *device)
{
return device->plat->context;
}
extern void gl_getclientsize(const struct gs_swap_chain *swap, uint32_t *width,
uint32_t *height)
{
xcb_connection_t *xcb_conn =
XGetXCBConnection(swap->device->plat->display);
xcb_window_t window = swap->wi->window;
xcb_get_geometry_reply_t *geometry =
get_window_geometry(xcb_conn, window);
if (geometry) {
*width = geometry->width;
*height = geometry->height;
}
free(geometry);
}
extern void gl_clear_context(gs_device_t *device)
{
Display *display = device->plat->display;
if (!glXMakeContextCurrent(display, None, None, NULL)) {
blog(LOG_ERROR, "Failed to reset current context.");
}
}
extern void gl_update(gs_device_t *device)
{
Display *display = device->plat->display;
xcb_window_t window = device->cur_swap->wi->window;
uint32_t values[] = {device->cur_swap->info.cx,
device->cur_swap->info.cy};
xcb_configure_window(XGetXCBConnection(display), window,
XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
values);
}
extern void device_load_swapchain(gs_device_t *device, gs_swapchain_t *swap)
{
if (device->cur_swap == swap)
return;
Display *dpy = device->plat->display;
GLXContext ctx = device->plat->context;
device->cur_swap = swap;
if (swap) {
XID window = swap->wi->window;
if (!glXMakeContextCurrent(dpy, window, window, ctx)) {
blog(LOG_ERROR, "Failed to make context current.");
}
} else {
GLXPbuffer pbuf = device->plat->pbuffer;
if (!glXMakeContextCurrent(dpy, pbuf, pbuf, ctx)) {
blog(LOG_ERROR, "Failed to make context current.");
}
}
}
enum swap_type {
SWAP_TYPE_NORMAL,
SWAP_TYPE_EXT,
SWAP_TYPE_MESA,
SWAP_TYPE_SGI,
};
extern void device_present(gs_device_t *device)
{
static bool initialized = false;
static enum swap_type swap_type = SWAP_TYPE_NORMAL;
Display *display = device->plat->display;
XID window = device->cur_swap->wi->window;
if (!initialized) {
if (GLAD_GLX_EXT_swap_control)
swap_type = SWAP_TYPE_EXT;
else if (GLAD_GLX_MESA_swap_control)
swap_type = SWAP_TYPE_MESA;
else if (GLAD_GLX_SGI_swap_control)
swap_type = SWAP_TYPE_SGI;
initialized = true;
}
xcb_connection_t *xcb_conn = XGetXCBConnection(display);
xcb_generic_event_t *xcb_event;
while ((xcb_event = xcb_poll_for_event(xcb_conn))) {
free(xcb_event);
}
switch (swap_type) {
case SWAP_TYPE_EXT:
glXSwapIntervalEXT(display, window, 0);
break;
case SWAP_TYPE_MESA:
glXSwapIntervalMESA(0);
break;
case SWAP_TYPE_SGI:
glXSwapIntervalSGI(0);
break;
case SWAP_TYPE_NORMAL:;
}
glXSwapBuffers(display, window);
}
