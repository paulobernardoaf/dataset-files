





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdarg.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_window.h>

static int Enable(vout_window_t *wnd, const vout_window_cfg_t *cfg)
{
vout_window_ReportSize(wnd, cfg->width, cfg->height);
return VLC_SUCCESS;
}

static const struct vout_window_operations ops = {
.enable = Enable,
.resize = vout_window_ReportSize,
};

static int Open(vout_window_t *wnd)
{
wnd->type = VOUT_WINDOW_TYPE_DUMMY;
wnd->ops = &ops;
return VLC_SUCCESS;
}

vlc_module_begin()
set_shortname(N_("Dummy window"))
set_description(N_("Dummy window"))
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VOUT)
set_capability("vout window", 1)
set_callback(Open)
add_shortcut("dummy")
vlc_module_end()
