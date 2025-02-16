#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include <vlc_opengl.h>
#include "vout_helper.h"
static int Open(vout_display_t *vd, const vout_display_cfg_t *cfg,
video_format_t *fmtp, vlc_video_context *context);
static void Close(vout_display_t *vd);
#define GL_TEXT N_("OpenGL extension")
#define GLES2_TEXT N_("OpenGL ES 2 extension")
#define PROVIDER_LONGTEXT N_( "Extension through which to use the Open Graphics Library (OpenGL).")
vlc_module_begin ()
#if defined (USE_OPENGL_ES2)
#define API VLC_OPENGL_ES2
#define MODULE_VARNAME "gles2"
set_shortname (N_("OpenGL ES2"))
set_description (N_("OpenGL for Embedded Systems 2 video output"))
set_callback_display(Open, 265)
add_shortcut ("opengles2", "gles2")
add_module("gles2", "opengl es2", NULL, GLES2_TEXT, PROVIDER_LONGTEXT)
#else
#define API VLC_OPENGL
#define MODULE_VARNAME "gl"
set_shortname (N_("OpenGL"))
set_description (N_("OpenGL video output"))
set_category (CAT_VIDEO)
set_subcategory (SUBCAT_VIDEO_VOUT)
set_callback_display(Open, 270)
add_shortcut ("opengl", "gl")
add_module("gl", "opengl", NULL, GL_TEXT, PROVIDER_LONGTEXT)
#endif
add_glopts ()
vlc_module_end ()
struct vout_display_sys_t
{
vout_display_opengl_t *vgl;
vlc_gl_t *gl;
vout_display_place_t place;
bool place_changed;
};
static void PictureRender (vout_display_t *, picture_t *, subpicture_t *, vlc_tick_t);
static void PictureDisplay (vout_display_t *, picture_t *);
static int Control (vout_display_t *, int, va_list);
static int Open(vout_display_t *vd, const vout_display_cfg_t *cfg,
video_format_t *fmt, vlc_video_context *context)
{
vout_display_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
sys->gl = NULL;
sys->place_changed = false;
vout_window_t *surface = cfg->window;
char *gl_name = var_InheritString(surface, MODULE_VARNAME);
#if !defined(USE_OPENGL_ES2)
if (surface->type == VOUT_WINDOW_TYPE_XID)
{
switch (fmt->i_chroma)
{
case VLC_CODEC_VDPAU_VIDEO_444:
case VLC_CODEC_VDPAU_VIDEO_422:
case VLC_CODEC_VDPAU_VIDEO_420:
{
if (gl_name == NULL || gl_name[0] == 0
|| strcmp(gl_name, "any") == 0)
{
free(gl_name);
gl_name = strdup("glx");
}
break;
}
default:
break;
}
}
#endif
sys->gl = vlc_gl_Create(cfg, API, gl_name);
free(gl_name);
if (sys->gl == NULL)
goto error;
const vlc_fourcc_t *spu_chromas;
if (vlc_gl_MakeCurrent (sys->gl))
goto error;
sys->vgl = vout_display_opengl_New (fmt, &spu_chromas, sys->gl,
&cfg->viewpoint, context);
vlc_gl_ReleaseCurrent (sys->gl);
if (sys->vgl == NULL)
goto error;
vd->sys = sys;
vd->info.subpicture_chromas = spu_chromas;
vd->prepare = PictureRender;
vd->display = PictureDisplay;
vd->control = Control;
vd->close = Close;
return VLC_SUCCESS;
error:
if (sys->gl != NULL)
vlc_gl_Release (sys->gl);
free (sys);
return VLC_EGENERIC;
}
static void Close(vout_display_t *vd)
{
vout_display_sys_t *sys = vd->sys;
vlc_gl_t *gl = sys->gl;
vlc_gl_MakeCurrent (gl);
vout_display_opengl_Delete (sys->vgl);
vlc_gl_ReleaseCurrent (gl);
vlc_gl_Release (gl);
free (sys);
}
static void PictureRender (vout_display_t *vd, picture_t *pic, subpicture_t *subpicture,
vlc_tick_t date)
{
VLC_UNUSED(date);
vout_display_sys_t *sys = vd->sys;
if (vlc_gl_MakeCurrent (sys->gl) == VLC_SUCCESS)
{
vout_display_opengl_Prepare (sys->vgl, pic, subpicture);
vlc_gl_ReleaseCurrent (sys->gl);
}
}
static void PictureDisplay (vout_display_t *vd, picture_t *pic)
{
vout_display_sys_t *sys = vd->sys;
VLC_UNUSED(pic);
if (vlc_gl_MakeCurrent (sys->gl) == VLC_SUCCESS)
{
if (sys->place_changed)
{
float window_ar = (float)sys->place.width / sys->place.height;
vout_display_opengl_SetWindowAspectRatio(sys->vgl, window_ar);
vout_display_opengl_Viewport(sys->vgl, sys->place.x, sys->place.y,
sys->place.width, sys->place.height);
sys->place_changed = false;
}
vout_display_opengl_Display(sys->vgl);
vlc_gl_ReleaseCurrent (sys->gl);
}
}
static int Control (vout_display_t *vd, int query, va_list ap)
{
vout_display_sys_t *sys = vd->sys;
switch (query)
{
#if !defined(NDEBUG)
case VOUT_DISPLAY_RESET_PICTURES: 
vlc_assert_unreachable();
#endif
case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE:
case VOUT_DISPLAY_CHANGE_DISPLAY_FILLED:
case VOUT_DISPLAY_CHANGE_ZOOM:
{
vout_display_cfg_t c = *va_arg (ap, const vout_display_cfg_t *);
const video_format_t *src = &vd->source;
if (c.align.vertical == VLC_VIDEO_ALIGN_TOP)
c.align.vertical = VLC_VIDEO_ALIGN_BOTTOM;
else if (c.align.vertical == VLC_VIDEO_ALIGN_BOTTOM)
c.align.vertical = VLC_VIDEO_ALIGN_TOP;
vout_display_PlacePicture(&sys->place, src, &c);
sys->place_changed = true;
vlc_gl_Resize (sys->gl, c.display.width, c.display.height);
return VLC_SUCCESS;
}
case VOUT_DISPLAY_CHANGE_SOURCE_ASPECT:
case VOUT_DISPLAY_CHANGE_SOURCE_CROP:
{
const vout_display_cfg_t *cfg = va_arg (ap, const vout_display_cfg_t *);
vout_display_PlacePicture(&sys->place, &vd->source, cfg);
sys->place_changed = true;
return VLC_SUCCESS;
}
case VOUT_DISPLAY_CHANGE_VIEWPOINT:
return vout_display_opengl_SetViewpoint (sys->vgl,
&va_arg (ap, const vout_display_cfg_t* )->viewpoint);
default:
msg_Err (vd, "Unknown request %d", query);
}
return VLC_EGENERIC;
}
