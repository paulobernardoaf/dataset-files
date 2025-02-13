



















#if !defined(VLC_GL_RENDERER_H)
#define VLC_GL_RENDERER_H

#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_opengl.h>
#include <vlc_plugin.h>

#include "gl_api.h"
#include "gl_common.h"
#include "interop.h"

struct pl_context;
struct pl_shader;
struct pl_shader_res;




struct vlc_gl_renderer
{

vlc_gl_t *gl;


struct pl_context *pl_ctx;


const struct vlc_gl_api *api;
const opengl_vtable_t *vt; 


bool b_dump_shaders;


unsigned glsl_version;


const char *glsl_precision_header;

GLuint program_id;

struct {
GLfloat OrientationMatrix[16];
GLfloat ProjectionMatrix[16];
GLfloat StereoMatrix[3*3];
GLfloat ZoomMatrix[16];
GLfloat ViewMatrix[16];

GLfloat TexCoordsMap[PICTURE_PLANE_MAX][3*3];
} var;

struct {
GLint Texture[PICTURE_PLANE_MAX];
GLint TexSize[PICTURE_PLANE_MAX]; 
GLint ConvMatrix;
GLint FillColor;
GLint *pl_vars; 

GLint TransformMatrix;
GLint OrientationMatrix;
GLint StereoMatrix;
GLint ProjectionMatrix;
GLint ViewMatrix;
GLint ZoomMatrix;

GLint TexCoordsMap[PICTURE_PLANE_MAX];
} uloc;

struct {
GLint PicCoordsIn;
GLint VertexPosition;
} aloc;

bool yuv_color;
GLfloat conv_matrix[16];

struct pl_shader *pl_sh;
const struct pl_shader_res *pl_sh_res;

struct vlc_gl_interop *interop;

video_format_t fmt;

GLsizei tex_width[PICTURE_PLANE_MAX];
GLsizei tex_height[PICTURE_PLANE_MAX];

GLuint textures[PICTURE_PLANE_MAX];

unsigned nb_indices;
GLuint vertex_buffer_object;
GLuint index_buffer_object;
GLuint texture_buffer_object;

struct {
unsigned int i_x_offset;
unsigned int i_y_offset;
unsigned int i_visible_width;
unsigned int i_visible_height;
} last_source;


vlc_viewpoint_t vp;
float f_teta;
float f_phi;
float f_roll;
float f_fovx; 
float f_fovy; 
float f_z; 
float f_sar;











int (*pf_fetch_locations)(struct vlc_gl_renderer *renderer, GLuint program);












void (*pf_prepare_shader)(const struct vlc_gl_renderer *renderer,
const GLsizei *tex_width, const GLsizei *tex_height,
float alpha);
};










struct vlc_gl_renderer *
vlc_gl_renderer_New(vlc_gl_t *gl, const struct vlc_gl_api *api,
vlc_video_context *context, const video_format_t *fmt,
bool dump_shaders);






void
vlc_gl_renderer_Delete(struct vlc_gl_renderer *renderer);










int
vlc_gl_renderer_Prepare(struct vlc_gl_renderer *renderer, picture_t *picture);






int
vlc_gl_renderer_Draw(struct vlc_gl_renderer *renderer);

int
vlc_gl_renderer_SetViewpoint(struct vlc_gl_renderer *renderer,
const vlc_viewpoint_t *p_vp);

void
vlc_gl_renderer_SetWindowAspectRatio(struct vlc_gl_renderer *renderer,
float f_sar);

#endif 
