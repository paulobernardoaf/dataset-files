#include <vlc_common.h>
#include <vlc_opengl.h>
#include <vlc_picture.h>
#include <vlc_picture_pool.h>
#include "gl_common.h"
struct vlc_gl_interop;
struct vlc_gl_interop_ops {
int
(*allocate_textures)(const struct vlc_gl_interop *interop,
GLuint textures[], const GLsizei tex_width[],
const GLsizei tex_height[]);
int
(*update_textures)(const struct vlc_gl_interop *interop,
GLuint textures[], const GLsizei tex_width[],
const GLsizei tex_height[], picture_t *pic,
const size_t plane_offsets[]);
const float *
(*get_transform_matrix)(const struct vlc_gl_interop *interop);
void
(*close)(struct vlc_gl_interop *interop);
};
struct vlc_gl_interop {
vlc_object_t obj;
module_t *module;
vlc_gl_t *gl;
const struct vlc_gl_api *api;
const opengl_vtable_t *vt; 
GLenum tex_target;
video_format_t fmt;
video_format_t sw_fmt;
vlc_video_context *vctx;
bool handle_texs_gen;
struct vlc_gl_tex_cfg {
vlc_rational_t w;
vlc_rational_t h;
GLint internal;
GLenum format;
GLenum type;
} texs[PICTURE_PLANE_MAX];
unsigned tex_count;
void *priv;
const struct vlc_gl_interop_ops *ops;
int
(*init)(struct vlc_gl_interop *interop, GLenum tex_target,
vlc_fourcc_t chroma, video_color_space_t yuv_space);
};
struct vlc_gl_interop *
vlc_gl_interop_New(struct vlc_gl_t *gl, const struct vlc_gl_api *api,
vlc_video_context *context, const video_format_t *fmt,
bool subpics);
void
vlc_gl_interop_Delete(struct vlc_gl_interop *interop);
int
vlc_gl_interop_GenerateTextures(const struct vlc_gl_interop *interop,
const GLsizei *tex_width,
const GLsizei *tex_height, GLuint *textures);
void
vlc_gl_interop_DeleteTextures(const struct vlc_gl_interop *interop,
GLuint *textures);
static inline int
opengl_interop_init(struct vlc_gl_interop *interop, GLenum tex_target,
vlc_fourcc_t chroma, video_color_space_t yuv_space)
{
return interop->init(interop, tex_target, chroma, yuv_space);
}
