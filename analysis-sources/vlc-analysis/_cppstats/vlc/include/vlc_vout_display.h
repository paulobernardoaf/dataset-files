#include <vlc_es.h>
#include <vlc_picture.h>
#include <vlc_picture_pool.h>
#include <vlc_subpicture.h>
#include <vlc_actions.h>
#include <vlc_mouse.h>
#include <vlc_vout.h>
#include <vlc_vout_window.h>
#include <vlc_viewpoint.h>
typedef struct vout_display_t vout_display_t;
typedef struct vout_display_sys_t vout_display_sys_t;
typedef struct vout_display_owner_t vout_display_owner_t;
#define VLC_VIDEO_ALIGN_CENTER 0
#define VLC_VIDEO_ALIGN_LEFT 1
#define VLC_VIDEO_ALIGN_RIGHT 2
#define VLC_VIDEO_ALIGN_TOP 1
#define VLC_VIDEO_ALIGN_BOTTOM 2
typedef struct vlc_video_align {
char horizontal;
char vertical;
} vlc_video_align_t;
typedef struct vout_display_cfg {
struct vout_window_t *window; 
#if defined(__OS2__)
bool is_fullscreen VLC_DEPRECATED; 
#endif
struct {
unsigned width; 
unsigned height; 
vlc_rational_t sar; 
} display;
struct {
unsigned width;
unsigned height;
} window_props;
vlc_video_align_t align;
bool is_display_filled;
struct {
unsigned num;
unsigned den;
} zoom;
vlc_viewpoint_t viewpoint;
} vout_display_cfg_t;
typedef struct {
bool can_scale_spu; 
const vlc_fourcc_t *subpicture_chromas; 
} vout_display_info_t;
enum vout_display_query {
VOUT_DISPLAY_RESET_PICTURES, 
#if defined(__OS2__)
VOUT_DISPLAY_CHANGE_FULLSCREEN VLC_DEPRECATED_ENUM, 
VOUT_DISPLAY_CHANGE_WINDOW_STATE VLC_DEPRECATED_ENUM, 
#endif
VOUT_DISPLAY_CHANGE_DISPLAY_SIZE, 
VOUT_DISPLAY_CHANGE_DISPLAY_FILLED, 
VOUT_DISPLAY_CHANGE_ZOOM, 
VOUT_DISPLAY_CHANGE_SOURCE_ASPECT, 
VOUT_DISPLAY_CHANGE_SOURCE_CROP, 
VOUT_DISPLAY_CHANGE_VIEWPOINT, 
};
struct vout_display_owner_t {
void *sys;
void (*viewpoint_moved)(void *sys, const vlc_viewpoint_t *vp);
};
typedef int (*vout_display_open_cb)(vout_display_t *vd,
const vout_display_cfg_t *cfg,
video_format_t *fmtp,
vlc_video_context *context);
#define set_callback_display(activate, priority) { vout_display_open_cb open__ = activate; (void) open__; set_callback(activate) } set_capability( "vout display", priority )
struct vout_display_t {
struct vlc_object_t obj;
const vout_display_cfg_t *cfg;
video_format_t source;
video_format_t fmt;
vout_display_info_t info;
void (*prepare)(vout_display_t *, picture_t *pic,
subpicture_t *subpic, vlc_tick_t date);
void (*display)(vout_display_t *, picture_t *pic);
int (*control)(vout_display_t *, int query, va_list);
void (*close)(vout_display_t *);
vout_display_sys_t *sys;
vout_display_owner_t owner;
};
VLC_API
vout_display_t *vout_display_New(vlc_object_t *,
const video_format_t *, vlc_video_context *,
const vout_display_cfg_t *, const char *module,
const vout_display_owner_t *);
VLC_API void vout_display_Delete(vout_display_t *);
VLC_API picture_t *vout_display_Prepare(vout_display_t *vd, picture_t *picture,
subpicture_t *subpic, vlc_tick_t date);
static inline void vout_display_Display(vout_display_t *vd, picture_t *picture)
{
if (vd->display != NULL)
vd->display(vd, picture);
picture_Release(picture);
}
VLC_API
void vout_display_SetSize(vout_display_t *vd, unsigned width, unsigned height);
VLC_API void vout_display_SendEventPicturesInvalid(vout_display_t *vd);
static inline void vout_display_SendEventMousePressed(vout_display_t *vd, int button)
{
vout_window_ReportMousePressed(vd->cfg->window, button);
}
static inline void vout_display_SendEventMouseReleased(vout_display_t *vd, int button)
{
vout_window_ReportMouseReleased(vd->cfg->window, button);
}
static inline void vout_display_SendEventMouseDoubleClick(vout_display_t *vd)
{
vout_window_ReportMouseDoubleClick(vd->cfg->window, MOUSE_BUTTON_LEFT);
}
static inline void vout_display_SendEventViewpointMoved(vout_display_t *vd,
const vlc_viewpoint_t *vp)
{
if (vd->owner.viewpoint_moved)
vd->owner.viewpoint_moved(vd->owner.sys, vp);
}
static inline void vout_display_SendMouseMovedDisplayCoordinates(vout_display_t *vd, int m_x, int m_y)
{
vout_window_ReportMouseMoved(vd->cfg->window, m_x, m_y);
}
static inline bool vout_display_cfg_IsWindowed(const vout_display_cfg_t *cfg)
{
return cfg->window->type != VOUT_WINDOW_TYPE_DUMMY;
}
VLC_API void vout_display_GetDefaultDisplaySize(unsigned *width, unsigned *height, const video_format_t *source, const vout_display_cfg_t *);
typedef struct {
int x; 
int y; 
unsigned width; 
unsigned height; 
} vout_display_place_t;
static inline bool vout_display_PlaceEquals(const vout_display_place_t *p1,
const vout_display_place_t *p2)
{
return p1->x == p2->x && p1->width == p2->width &&
p1->y == p2->y && p1->height == p2->height;
}
VLC_API void vout_display_PlacePicture(vout_display_place_t *place, const video_format_t *source, const vout_display_cfg_t *cfg);
void vout_display_TranslateMouseState(vout_display_t *vd, vlc_mouse_t *video,
const vlc_mouse_t *window);
