
#if !defined(__al_included_allegro5_gl_ext_h)
#define __al_included_allegro5_gl_ext_h

#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)






#if defined(ALLEGRO_MSVC)
#undef GL_EXT_vertex_array
#undef GL_EXT_paletted_texture
#undef GL_WIN_swap_hint
#undef GL_WIN_draw_range_elements
#endif









#if !defined ALLEGRO_CFG_OPENGLES
#include "allegro5/opengl/GLext/gl_ext_defs.h"
#endif
#if defined ALLEGRO_WINDOWS && !defined ALLEGRO_EXCLUDE_WGL
#include "allegro5/opengl/GLext/wgl_ext_defs.h"
#elif defined ALLEGRO_UNIX && !defined ALLEGRO_EXCLUDE_GLX
#include "allegro5/opengl/GLext/glx_ext_defs.h"
#endif









#if !defined(APIENTRY)
#define APIENTRY
#define APIENTRY_defined
#endif

#define AGL_API(type, name, args) typedef type (APIENTRY * _ALLEGRO_gl##name##_t) args;
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) typedef type (APIENTRY * _ALLEGRO_wgl##name##_t) args;
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) typedef type (APIENTRY * _ALLEGRO_glX##name##_t) args;
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#endif

#if defined(APIENTRY_defined)
#undef APIENTRY
#undef APIENTRY_defined
#endif










#define AGL_API(type, name, args) AL_VAR(_ALLEGRO_gl##name##_t, _al_gl##name);
#include "allegro5/opengl/GLext/gl_ext_alias.h"
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) AL_VAR(_ALLEGRO_wgl##name##_t, _al_wgl##name);
#include "allegro5/opengl/GLext/wgl_ext_alias.h"
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) extern _ALLEGRO_glX##name##_t _al_glX##name;
#include "allegro5/opengl/GLext/glx_ext_alias.h"
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#endif








typedef struct ALLEGRO_OGL_EXT_LIST {
#define AGL_EXT(name, ver) int ALLEGRO_GL_##name;
#include "allegro5/opengl/GLext/gl_ext_list.h"
#undef AGL_EXT

#if defined(ALLEGRO_UNIX)
#define AGL_EXT(name, ver) int ALLEGRO_GLX_##name;
#include "allegro5/opengl/GLext/glx_ext_list.h"
#undef AGL_EXT
#elif defined ALLEGRO_WINDOWS
#define AGL_EXT(name, ver) int ALLEGRO_WGL_##name;
#include "allegro5/opengl/GLext/wgl_ext_list.h"
#undef AGL_EXT

#endif
} ALLEGRO_OGL_EXT_LIST;








typedef struct ALLEGRO_OGL_EXT_API {
#define AGL_API(type, name, args) _ALLEGRO_gl##name##_t name;
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) _ALLEGRO_wgl##name##_t name;
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) _ALLEGRO_glX##name##_t name;
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#endif
} ALLEGRO_OGL_EXT_API;

#else

typedef struct ALLEGRO_OGL_EXT_LIST ALLEGRO_OGL_EXT_LIST;

#endif

#endif

