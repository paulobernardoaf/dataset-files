#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdarg.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_window.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_picture.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_renderer_discoverer.h>
#include <vlc/libvlc_media_player.h>
static int Open(vout_window_t *);
vlc_module_begin()
set_shortname(N_("Callback window"))
set_description(N_("External callback window"))
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VOUT)
set_capability("vout window", 0)
set_callback(Open)
vlc_module_end()
typedef struct {
void *opaque;
libvlc_video_output_set_resize_cb setResizeCb;
} wextern_t;
static void WindowResize(void *opaque, unsigned width, unsigned height)
{
vout_window_t *window = opaque;
vout_window_ReportSize(window, width, height);
}
static int Enable(struct vout_window_t *wnd, const vout_window_cfg_t *wcfg)
{
wextern_t *sys = wnd->sys;
if ( sys->setResizeCb != NULL )
sys->setResizeCb( sys->opaque, WindowResize, wnd );
(void) wcfg;
return VLC_SUCCESS;
}
static void Disable(struct vout_window_t *wnd)
{
wextern_t *sys = wnd->sys;
if ( sys->setResizeCb != NULL )
sys->setResizeCb( sys->opaque, NULL, NULL );
}
static const struct vout_window_operations ops = {
.enable = Enable,
.disable = Disable,
};
static int Open(vout_window_t *wnd)
{
wextern_t *sys = vlc_obj_malloc(VLC_OBJECT(wnd), sizeof(*sys));
if (unlikely(sys==NULL))
return VLC_ENOMEM;
sys->opaque = var_InheritAddress( wnd, "vout-cb-opaque" );
sys->setResizeCb = var_InheritAddress( wnd, "vout-cb-resize-cb" );
wnd->sys = sys;
wnd->type = VOUT_WINDOW_TYPE_DUMMY;
wnd->ops = &ops;
return VLC_SUCCESS;
}
