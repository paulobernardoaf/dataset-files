#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_opengl.h>
#include "gl_api.h"
#include "gl_common.h"
struct vlc_gl_sub_renderer;
struct vlc_gl_sub_renderer *
vlc_gl_sub_renderer_New(vlc_gl_t *gl, const struct vlc_gl_api *api);
void
vlc_gl_sub_renderer_Delete(struct vlc_gl_sub_renderer *sr);
int
vlc_gl_sub_renderer_Prepare(struct vlc_gl_sub_renderer *sr,
subpicture_t *subpicture);
int
vlc_gl_sub_renderer_Draw(struct vlc_gl_sub_renderer *sr);
