#include "allegro5/opengl/gl_ext.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_display.h"
#if defined(__cplusplus)
extern "C" {
#endif
enum {
_ALLEGRO_OPENGL_VERSION_0 = 0, 
_ALLEGRO_OPENGL_VERSION_1_0 = 0x01000000,
_ALLEGRO_OPENGL_VERSION_1_1 = 0x01010000,
_ALLEGRO_OPENGL_VERSION_1_2 = 0x01020000,
_ALLEGRO_OPENGL_VERSION_1_2_1 = 0x01020100,
_ALLEGRO_OPENGL_VERSION_1_3 = 0x01030000,
_ALLEGRO_OPENGL_VERSION_1_4 = 0x01040000,
_ALLEGRO_OPENGL_VERSION_1_5 = 0x01050000,
_ALLEGRO_OPENGL_VERSION_2_0 = 0x02000000,
_ALLEGRO_OPENGL_VERSION_2_1 = 0x02010000,
_ALLEGRO_OPENGL_VERSION_3_0 = 0x03000000,
_ALLEGRO_OPENGL_VERSION_3_1 = 0x03010000,
_ALLEGRO_OPENGL_VERSION_3_2 = 0x03020000,
_ALLEGRO_OPENGL_VERSION_3_3 = 0x03030000,
_ALLEGRO_OPENGL_VERSION_4_0 = 0x04000000
};
#define ALLEGRO_MAX_OPENGL_FBOS 8
enum {
FBO_INFO_UNUSED = 0,
FBO_INFO_TRANSIENT = 1, 
FBO_INFO_PERSISTENT = 2 
};
typedef struct ALLEGRO_FBO_BUFFERS
{
GLuint depth_buffer;
int dw, dh, depth;
GLuint multisample_buffer;
int mw, mh, samples;
} ALLEGRO_FBO_BUFFERS;
typedef struct ALLEGRO_FBO_INFO
{
int fbo_state;
GLuint fbo;
ALLEGRO_FBO_BUFFERS buffers;
ALLEGRO_BITMAP *owner;
double last_use_time;
} ALLEGRO_FBO_INFO;
typedef struct ALLEGRO_BITMAP_EXTRA_OPENGL
{
int true_w;
int true_h;
GLuint texture; 
ALLEGRO_FBO_INFO *fbo_info;
unsigned char *lock_buffer;
ALLEGRO_BITMAP *lock_proxy;
float left, top, right, bottom; 
bool is_backbuffer; 
} ALLEGRO_BITMAP_EXTRA_OPENGL;
typedef struct OPENGL_INFO {
uint32_t version; 
int max_texture_size; 
int is_voodoo3_and_under; 
int is_voodoo; 
int is_matrox_g200; 
int is_ati_rage_pro; 
int is_ati_radeon_7000; 
int is_ati_r200_chip; 
int is_mesa_driver; 
int is_intel_hd_graphics_3000;
} OPENGL_INFO;
typedef struct ALLEGRO_OGL_VARLOCS
{
GLint pos_loc;
GLint color_loc;
GLint projview_matrix_loc;
GLint texcoord_loc;
GLint use_tex_loc;
GLint tex_loc;
GLint use_tex_matrix_loc;
GLint tex_matrix_loc;
GLint alpha_test_loc;
GLint alpha_func_loc;
GLint alpha_test_val_loc;
GLint user_attr_loc[_ALLEGRO_PRIM_MAX_USER_ATTR];
} ALLEGRO_OGL_VARLOCS;
typedef struct ALLEGRO_OGL_EXTRAS
{
ALLEGRO_OGL_EXT_LIST *extension_list;
ALLEGRO_OGL_EXT_API *extension_api;
OPENGL_INFO ogl_info;
ALLEGRO_BITMAP *opengl_target;
ALLEGRO_BITMAP *backbuffer;
bool is_shared;
ALLEGRO_FBO_INFO fbos[ALLEGRO_MAX_OPENGL_FBOS];
GLuint program_object;
ALLEGRO_OGL_VARLOCS varlocs;
GLuint vao, vbo;
} ALLEGRO_OGL_EXTRAS;
typedef struct ALLEGRO_OGL_BITMAP_VERTEX
{
float x, y, z;
float tx, ty;
float r, g, b, a;
} ALLEGRO_OGL_BITMAP_VERTEX;
int _al_ogl_look_for_an_extension(const char *name, const GLubyte *extensions);
void _al_ogl_set_extensions(ALLEGRO_OGL_EXT_API *ext);
void _al_ogl_manage_extensions(ALLEGRO_DISPLAY *disp);
void _al_ogl_unmanage_extensions(ALLEGRO_DISPLAY *disp);
int _al_ogl_get_glformat(int format, int component);
ALLEGRO_BITMAP *_al_ogl_create_bitmap(ALLEGRO_DISPLAY *d, int w, int h,
int format, int flags);
void _al_ogl_upload_bitmap_memory(ALLEGRO_BITMAP *bitmap, int format, void *ptr);
#if !defined(ALLEGRO_CFG_OPENGLES)
ALLEGRO_LOCKED_REGION *_al_ogl_lock_region_new(ALLEGRO_BITMAP *bitmap,
int x, int y, int w, int h, int format, int flags);
void _al_ogl_unlock_region_new(ALLEGRO_BITMAP *bitmap);
#else
ALLEGRO_LOCKED_REGION *_al_ogl_lock_region_gles(ALLEGRO_BITMAP *bitmap,
int x, int y, int w, int h, int format, int flags);
void _al_ogl_unlock_region_gles(ALLEGRO_BITMAP *bitmap);
#endif
int _al_ogl_pixel_alignment(int pixel_size, bool compressed);
GLint _al_ogl_bind_framebuffer(GLint fbo);
void _al_ogl_reset_fbo_info(ALLEGRO_FBO_INFO *info);
bool _al_ogl_create_persistent_fbo(ALLEGRO_BITMAP *bitmap);
ALLEGRO_FBO_INFO *_al_ogl_persist_fbo(ALLEGRO_DISPLAY *display,
ALLEGRO_FBO_INFO *transient_fbo_info);
void _al_ogl_setup_fbo(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap);
bool _al_ogl_setup_fbo_non_backbuffer(ALLEGRO_DISPLAY *display,
ALLEGRO_BITMAP *bitmap);
void _al_ogl_del_fbo(ALLEGRO_FBO_INFO *info);
void _al_ogl_setup_gl(ALLEGRO_DISPLAY *d);
void _al_ogl_set_target_bitmap(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap);
void _al_ogl_unset_target_bitmap(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap);
void _al_ogl_finalize_fbo(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap);
void _al_ogl_setup_bitmap_clipping(const ALLEGRO_BITMAP *bitmap);
ALLEGRO_BITMAP *_al_ogl_get_backbuffer(ALLEGRO_DISPLAY *d);
ALLEGRO_BITMAP* _al_ogl_create_backbuffer(ALLEGRO_DISPLAY *disp);
void _al_ogl_destroy_backbuffer(ALLEGRO_BITMAP *b);
bool _al_ogl_resize_backbuffer(ALLEGRO_BITMAP *b, int w, int h);
void _al_opengl_backup_dirty_bitmaps(ALLEGRO_DISPLAY *d, bool flip);
struct ALLEGRO_DISPLAY_INTERFACE;
void _al_ogl_add_drawing_functions(struct ALLEGRO_DISPLAY_INTERFACE *vt);
AL_FUNC(bool, _al_opengl_set_blender, (ALLEGRO_DISPLAY *disp));
AL_FUNC(char const *, _al_gl_error_string, (GLenum e));
void _al_ogl_update_render_state(ALLEGRO_DISPLAY *display);
#if defined(ALLEGRO_CFG_SHADER_GLSL)
bool _al_glsl_set_projview_matrix(GLint projview_matrix_loc,
const ALLEGRO_TRANSFORM *t);
void _al_glsl_init_shaders(void);
void _al_glsl_shutdown_shaders(void);
void _al_glsl_unuse_shaders(void);
#endif
#if defined(__cplusplus)
}
#endif
