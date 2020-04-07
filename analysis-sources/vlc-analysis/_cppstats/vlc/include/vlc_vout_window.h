#include <stdarg.h>
#include <vlc_common.h>
struct vout_window_t;
struct wl_display;
struct wl_surface;
enum vout_window_type {
VOUT_WINDOW_TYPE_DUMMY ,
VOUT_WINDOW_TYPE_XID ,
VOUT_WINDOW_TYPE_HWND ,
VOUT_WINDOW_TYPE_NSOBJECT ,
VOUT_WINDOW_TYPE_ANDROID_NATIVE ,
VOUT_WINDOW_TYPE_WAYLAND ,
};
enum vout_window_state {
VOUT_WINDOW_STATE_NORMAL ,
VOUT_WINDOW_STATE_ABOVE ,
VOUT_WINDOW_STATE_BELOW ,
};
enum vout_window_mouse_event_type {
VOUT_WINDOW_MOUSE_MOVED ,
VOUT_WINDOW_MOUSE_PRESSED ,
VOUT_WINDOW_MOUSE_RELEASED ,
VOUT_WINDOW_MOUSE_DOUBLE_CLICK ,
};
typedef struct vout_window_mouse_event_t
{
enum vout_window_mouse_event_type type; 
int x;
int y;
int button_mask;
} vout_window_mouse_event_t;
typedef struct vout_window_cfg_t {
bool is_fullscreen;
bool is_decorated;
#if defined(__APPLE__) || defined(_WIN32)
int x;
int y;
#endif
unsigned width;
unsigned height;
} vout_window_cfg_t;
struct vout_window_callbacks {
void (*resized)(struct vout_window_t *, unsigned width, unsigned height);
void (*closed)(struct vout_window_t *);
void (*state_changed)(struct vout_window_t *, unsigned state);
void (*windowed)(struct vout_window_t *);
void (*fullscreened)(struct vout_window_t *, const char *id);
void (*mouse_event)(struct vout_window_t *,
const vout_window_mouse_event_t *mouse);
void (*keyboard_event)(struct vout_window_t *, unsigned key);
void (*output_event)(struct vout_window_t *,
const char *id, const char *desc);
};
typedef struct vout_window_owner {
const struct vout_window_callbacks *cbs; 
void *sys; 
} vout_window_owner_t;
struct vout_window_operations {
int (*enable)(struct vout_window_t *, const vout_window_cfg_t *);
void (*disable)(struct vout_window_t *);
void (*resize)(struct vout_window_t *, unsigned width, unsigned height);
void (*destroy)(struct vout_window_t *);
void (*set_state)(struct vout_window_t *, unsigned state);
void (*unset_fullscreen)(struct vout_window_t *);
void (*set_fullscreen)(struct vout_window_t *, const char *id);
void (*set_title)(struct vout_window_t *, const char *id);
};
typedef struct vout_window_t {
struct vlc_object_t obj;
unsigned type;
union {
void *hwnd; 
uint32_t xid; 
void *nsobject; 
void *anativewindow; 
struct wl_surface *wl; 
} handle;
union {
char *x11; 
struct wl_display *wl; 
} display;
const struct vout_window_operations *ops; 
struct {
bool has_double_click; 
} info;
void *sys;
vout_window_owner_t owner;
} vout_window_t;
VLC_API vout_window_t *vout_window_New(vlc_object_t *obj,
const char *module,
const vout_window_owner_t *owner);
VLC_API void vout_window_Delete(vout_window_t *window);
void vout_window_SetInhibition(vout_window_t *window, bool enabled);
static inline void vout_window_SetState(vout_window_t *window, unsigned state)
{
if (window->ops->set_state != NULL)
window->ops->set_state(window, state);
}
static inline void vout_window_SetSize(vout_window_t *window,
unsigned width, unsigned height)
{
if (window->ops->resize != NULL)
window->ops->resize(window, width, height);
}
static inline void vout_window_SetFullScreen(vout_window_t *window,
const char *id)
{
if (window->ops->set_fullscreen != NULL)
window->ops->set_fullscreen(window, id);
}
static inline void vout_window_UnsetFullScreen(vout_window_t *window)
{
if (window->ops->unset_fullscreen != NULL)
window->ops->unset_fullscreen(window);
}
static inline void vout_window_SetTitle(vout_window_t *window, const char *title)
{
if (window->ops->set_title != NULL)
window->ops->set_title(window, title);
}
VLC_API
int vout_window_Enable(vout_window_t *window, const vout_window_cfg_t *cfg);
VLC_API
void vout_window_Disable(vout_window_t *window);
static inline void vout_window_ReportSize(vout_window_t *window,
unsigned width, unsigned height)
{
window->owner.cbs->resized(window, width, height);
}
static inline void vout_window_ReportClose(vout_window_t *window)
{
if (window->owner.cbs->closed != NULL)
window->owner.cbs->closed(window);
}
static inline void vout_window_ReportState(vout_window_t *window,
unsigned state)
{
if (window->owner.cbs->state_changed != NULL)
window->owner.cbs->state_changed(window, state);
}
VLC_API void vout_window_ReportWindowed(vout_window_t *wnd);
VLC_API void vout_window_ReportFullscreen(vout_window_t *wnd, const char *id);
static inline void vout_window_SendMouseEvent(vout_window_t *window,
const vout_window_mouse_event_t *mouse)
{
if (window->owner.cbs->mouse_event != NULL)
window->owner.cbs->mouse_event(window, mouse);
}
static inline void vout_window_ReportMouseMoved(vout_window_t *window,
int x, int y)
{
const vout_window_mouse_event_t mouse = {
VOUT_WINDOW_MOUSE_MOVED, x, y, 0
};
vout_window_SendMouseEvent(window, &mouse);
}
static inline void vout_window_ReportMousePressed(vout_window_t *window,
int button)
{
const vout_window_mouse_event_t mouse = {
VOUT_WINDOW_MOUSE_PRESSED, 0, 0, button,
};
vout_window_SendMouseEvent(window, &mouse);
}
static inline void vout_window_ReportMouseReleased(vout_window_t *window,
int button)
{
const vout_window_mouse_event_t mouse = {
VOUT_WINDOW_MOUSE_RELEASED, 0, 0, button,
};
vout_window_SendMouseEvent(window, &mouse);
}
static inline void vout_window_ReportMouseDoubleClick(vout_window_t *window,
int button)
{
const vout_window_mouse_event_t mouse = {
VOUT_WINDOW_MOUSE_DOUBLE_CLICK, 0, 0, button,
};
vout_window_SendMouseEvent(window, &mouse);
}
static inline void vout_window_ReportKeyPress(vout_window_t *window, int key)
{
if (window->owner.cbs->keyboard_event != NULL)
window->owner.cbs->keyboard_event(window, key);
}
static inline void vout_window_ReportOutputDevice(vout_window_t *window,
const char *id,
const char *name)
{
if (window->owner.cbs->output_event != NULL)
window->owner.cbs->output_event(window, id, name);
}
