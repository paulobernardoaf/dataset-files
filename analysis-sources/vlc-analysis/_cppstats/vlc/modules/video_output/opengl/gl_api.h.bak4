




















#if !defined(VLC_GL_API_H)
#define VLC_GL_API_H

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdbool.h>
#include <vlc_common.h>
#include <vlc_opengl.h>

#include "gl_common.h"

struct vlc_gl_api {
opengl_vtable_t vt;


bool is_gles;


const char *extensions;


bool supports_npot;
};

int
vlc_gl_api_Init(struct vlc_gl_api *api, vlc_gl_t *gl);

#endif
