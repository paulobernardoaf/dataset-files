#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#if !defined(__ANDROID__)
#error this file must be built from android
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include "interop.h"
#include "../android/utils.h"
struct priv
{
android_video_context_t *avctx;
AWindowHandler *awh;
const float *transform_mtx;
bool stex_attached;
};
static int
tc_anop_allocate_textures(const struct vlc_gl_interop *interop, GLuint *textures,
const GLsizei *tex_width, const GLsizei *tex_height)
{
(void) tex_width; (void) tex_height;
struct priv *priv = interop->priv;
assert(textures[0] != 0);
if (SurfaceTexture_attachToGLContext(priv->awh, textures[0]) != 0)
{
msg_Err(interop->gl, "SurfaceTexture_attachToGLContext failed");
return VLC_EGENERIC;
}
priv->stex_attached = true;
return VLC_SUCCESS;
}
static int
tc_anop_update(const struct vlc_gl_interop *interop, GLuint *textures,
const GLsizei *tex_width, const GLsizei *tex_height,
picture_t *pic, const size_t *plane_offset)
{
(void) tex_width; (void) tex_height; (void) plane_offset;
assert(pic->context);
assert(textures[0] != 0);
if (plane_offset != NULL)
return VLC_EGENERIC;
struct priv *priv = interop->priv;
if (!priv->avctx->render(pic->context))
return VLC_SUCCESS; 
if (SurfaceTexture_waitAndUpdateTexImage(priv->awh, &priv->transform_mtx)
!= VLC_SUCCESS)
{
priv->transform_mtx = NULL;
return VLC_EGENERIC;
}
interop->vt->ActiveTexture(GL_TEXTURE0);
interop->vt->BindTexture(interop->tex_target, textures[0]);
return VLC_SUCCESS;
}
static const float *
tc_get_transform_matrix(const struct vlc_gl_interop *interop)
{
struct priv *priv = interop->priv;
return priv->transform_mtx;
}
static void
Close(struct vlc_gl_interop *interop)
{
struct priv *priv = interop->priv;
if (priv->stex_attached)
SurfaceTexture_detachFromGLContext(priv->awh);
free(priv);
}
static int
Open(vlc_object_t *obj)
{
struct vlc_gl_interop *interop = (void *) obj;
if (interop->fmt.i_chroma != VLC_CODEC_ANDROID_OPAQUE
|| !interop->gl->surface->handle.anativewindow
|| !interop->vctx)
return VLC_EGENERIC;
android_video_context_t *avctx =
vlc_video_context_GetPrivate(interop->vctx, VLC_VIDEO_CONTEXT_AWINDOW);
if (avctx->id != AWindow_SurfaceTexture)
return VLC_EGENERIC;
interop->priv = malloc(sizeof(struct priv));
if (unlikely(interop->priv == NULL))
return VLC_ENOMEM;
struct priv *priv = interop->priv;
priv->avctx = avctx;
priv->awh = interop->gl->surface->handle.anativewindow;
priv->transform_mtx = NULL;
priv->stex_attached = false;
static const struct vlc_gl_interop_ops ops = {
.allocate_textures = tc_anop_allocate_textures,
.update_textures = tc_anop_update,
.get_transform_matrix = tc_get_transform_matrix,
.close = Close,
};
interop->ops = &ops;
switch (interop->fmt.orientation)
{
case ORIENT_TOP_LEFT:
interop->fmt.orientation = ORIENT_BOTTOM_LEFT;
break;
case ORIENT_TOP_RIGHT:
interop->fmt.orientation = ORIENT_BOTTOM_RIGHT;
break;
case ORIENT_BOTTOM_LEFT:
interop->fmt.orientation = ORIENT_TOP_LEFT;
break;
case ORIENT_BOTTOM_RIGHT:
interop->fmt.orientation = ORIENT_TOP_RIGHT;
break;
case ORIENT_LEFT_TOP:
interop->fmt.orientation = ORIENT_RIGHT_TOP;
break;
case ORIENT_LEFT_BOTTOM:
interop->fmt.orientation = ORIENT_RIGHT_BOTTOM;
break;
case ORIENT_RIGHT_TOP:
interop->fmt.orientation = ORIENT_LEFT_TOP;
break;
case ORIENT_RIGHT_BOTTOM:
interop->fmt.orientation = ORIENT_LEFT_BOTTOM;
break;
}
int ret = opengl_interop_init(interop, GL_TEXTURE_EXTERNAL_OES,
VLC_CODEC_RGB32,
COLOR_SPACE_UNDEF);
if (ret != VLC_SUCCESS)
{
free(priv);
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
vlc_module_begin ()
set_description("Android OpenGL SurfaceTexture converter")
set_capability("glinterop", 1)
set_callback(Open)
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VOUT)
vlc_module_end ()
