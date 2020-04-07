#include "gl-subsystem.h"
static bool gl_init_zsbuffer(struct gs_zstencil_buffer *zs, uint32_t width,
uint32_t height)
{
glGenRenderbuffers(1, &zs->buffer);
if (!gl_success("glGenRenderbuffers"))
return false;
if (!gl_bind_renderbuffer(GL_RENDERBUFFER, zs->buffer))
return false;
glRenderbufferStorage(GL_RENDERBUFFER, zs->format, width, height);
if (!gl_success("glRenderbufferStorage"))
return false;
gl_bind_renderbuffer(GL_RENDERBUFFER, 0);
return true;
}
static inline GLenum get_attachment(enum gs_zstencil_format format)
{
switch (format) {
case GS_Z16:
return GL_DEPTH_ATTACHMENT;
case GS_Z24_S8:
return GL_DEPTH_STENCIL_ATTACHMENT;
case GS_Z32F:
return GL_DEPTH_ATTACHMENT;
case GS_Z32F_S8X24:
return GL_DEPTH_STENCIL_ATTACHMENT;
case GS_ZS_NONE:
return 0;
}
return 0;
}
gs_zstencil_t *device_zstencil_create(gs_device_t *device, uint32_t width,
uint32_t height,
enum gs_zstencil_format format)
{
struct gs_zstencil_buffer *zs;
zs = bzalloc(sizeof(struct gs_zstencil_buffer));
zs->format = convert_zstencil_format(format);
zs->attachment = get_attachment(format);
zs->device = device;
if (!gl_init_zsbuffer(zs, width, height)) {
blog(LOG_ERROR, "device_zstencil_create (GL) failed");
gs_zstencil_destroy(zs);
return NULL;
}
return zs;
}
void gs_zstencil_destroy(gs_zstencil_t *zs)
{
if (zs) {
if (zs->buffer) {
glDeleteRenderbuffers(1, &zs->buffer);
gl_success("glDeleteRenderbuffers");
}
bfree(zs);
}
}
