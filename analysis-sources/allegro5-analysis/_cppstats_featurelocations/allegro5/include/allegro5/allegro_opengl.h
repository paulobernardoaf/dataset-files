
















#if !defined(__al_included_allegro5_allegro_opengl_h)
#define __al_included_allegro5_allegro_opengl_h

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(ALLEGRO_WINDOWS)
#include <windows.h>
#endif

#if defined ALLEGRO_IPHONE

#if defined(ALLEGRO_CFG_OPENGLES1)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(ALLEGRO_CFG_OPENGLES3)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(ALLEGRO_CFG_OPENGLES2)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#if defined(ALLEGRO_CFG_OPENGLES1)



#define glBlendEquation glBlendEquationOES
#define glBlendFuncSeparate glBlendFuncSeparateOES
#define glBlendEquationSeparate glBlendEquationSeparateOES
#define glRenderbufferStorageMultisampleEXT glRenderbufferStorageMultisampleAPPLE
#if defined(GL_FUNC_ADD)
#undef GL_FUNC_ADD
#undef GL_FUNC_SUBTRACT
#undef GL_FUNC_REVERSE_SUBTRACT
#endif
#define GL_FUNC_ADD GL_FUNC_ADD_OES
#define GL_FUNC_SUBTRACT GL_FUNC_SUBTRACT_OES
#define GL_FUNC_REVERSE_SUBTRACT GL_FUNC_REVERSE_SUBTRACT_OES

#elif defined(ALLEGRO_CFG_OPENGLES3)

#define glRenderbufferStorageMultisampleEXT glRenderbufferStorageMultisample

#endif

#elif defined(ALLEGRO_MACOSX)

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#if !defined(GL_GLEXT_PROTOTYPES)
#define GL_GLEXT_PROTOTYPES
#endif

#elif defined(ALLEGRO_CFG_OPENGLES1)

#include <GLES/gl.h>
#include <GLES/glext.h>

#define GL_FUNC_ADD GL_FUNC_ADD_OES
#define GL_FUNC_SUBTRACT GL_FUNC_SUBTRACT_OES
#define GL_FUNC_REVERSE_SUBTRACT GL_FUNC_REVERSE_SUBTRACT_OES

#define GL_FRAMEBUFFER_BINDING_EXT GL_FRAMEBUFFER_BINDING_OES
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER_OES
#define glBlendEquation glBlendEquationOES
#define glBlendFuncSeparate glBlendFuncSeparateOES
#define glBlendEquationSeparate glBlendEquationSeparateOES
#define glGenerateMipmapEXT glGenerateMipmapOES
#define glBindFramebufferEXT glBindFramebufferOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define GL_DEPTH_COMPONENT16 GL_DEPTH_COMPONENT16_OES
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES

#elif defined(ALLEGRO_CFG_OPENGLES3)

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>




#define glRenderbufferStorageMultisampleEXT glRenderbufferStorageMultisample

#elif defined(ALLEGRO_CFG_OPENGLES2)

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define GL_RGBA8 GL_RGBA8_OES

#define GL_FRAMEBUFFER_BINDING_EXT GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define glGenerateMipmapEXT glGenerateMipmap
#define glBindFramebufferEXT glBindFramebuffer
#define glDeleteFramebuffersEXT glDeleteFramebuffers

#else


#define __glext_h_
#define __gl_glext_h_
#define __glxext_h_
#define __glx_glxext_h_
#include <GL/gl.h>
#undef __glext_h_
#undef __gl_glext_h_
#undef __glxext_h_
#undef __glx_glxext_h_

#endif

#if defined(ALLEGRO_RASPBERRYPI)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include "allegro5/bitmap.h"
#include "allegro5/display.h"
#include "allegro5/shader.h"
#include "allegro5/opengl/gl_ext.h"

#if defined(ALLEGRO_WINDOWS)


#if !defined(PFD_SWAP_LAYER_BUFFERS)
#define PFD_SWAP_LAYER_BUFFERS 0x00000800
#endif

#if !defined(PFD_GENERIC_ACCELERATED)
#define PFD_GENERIC_ACCELERATED 0x00001000
#endif

#if !defined(PFD_SUPPORT_DIRECTDRAW)
#define PFD_SUPPORT_DIRECTDRAW 0x00002000
#endif

#if !defined(CDS_FULLSCREEN)
#define CDS_FULLSCREEN 0x00000004
#endif

#if !defined(ENUM_CURRENT_SETTINGS)
#define ENUM_CURRENT_SETTINGS ((DWORD)-1)
#endif

#endif 

#if defined ALLEGRO_WINDOWS
#define ALLEGRO_DEFINE_PROC_TYPE(type, name, args) typedef type (APIENTRY * name) args;

#else
#define ALLEGRO_DEFINE_PROC_TYPE(type, name, args) typedef type (*name) args;

#endif







typedef enum ALLEGRO_OPENGL_VARIANT {
ALLEGRO_DESKTOP_OPENGL = 0,
ALLEGRO_OPENGL_ES
} ALLEGRO_OPENGL_VARIANT;

AL_FUNC(uint32_t, al_get_opengl_version, (void));
AL_FUNC(bool, al_have_opengl_extension, (const char *extension));
AL_FUNC(void*, al_get_opengl_proc_address, (const char *name));
AL_FUNC(ALLEGRO_OGL_EXT_LIST*, al_get_opengl_extension_list, (void));
AL_FUNC(GLuint, al_get_opengl_texture, (ALLEGRO_BITMAP *bitmap));
AL_FUNC(void, al_remove_opengl_fbo, (ALLEGRO_BITMAP *bitmap));
AL_FUNC(GLuint, al_get_opengl_fbo, (ALLEGRO_BITMAP *bitmap));
AL_FUNC(bool, al_get_opengl_texture_size, (ALLEGRO_BITMAP *bitmap,
int *w, int *h));
AL_FUNC(void, al_get_opengl_texture_position, (ALLEGRO_BITMAP *bitmap,
int *u, int *v));
AL_FUNC(GLuint, al_get_opengl_program_object, (ALLEGRO_SHADER *shader));
AL_FUNC(void, al_set_current_opengl_context, (ALLEGRO_DISPLAY *display));
AL_FUNC(int, al_get_opengl_variant, (void));

#if defined(__cplusplus)
}
#endif

#endif
