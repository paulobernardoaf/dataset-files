#include <math.h>
#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_opengl.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_memblit.h"
#include "allegro5/internal/aintern_opengl.h"
#include "allegro5/internal/aintern_pixels.h"
#include "allegro5/internal/aintern_system.h"
#include "allegro5/internal/aintern_transform.h"
#if defined ALLEGRO_ANDROID
#include "allegro5/internal/aintern_android.h"
#endif
#include "ogl_helpers.h"
ALLEGRO_DEBUG_CHANNEL("opengl")
#define get_glformat(f, c) _al_ogl_get_glformat((f), (c))
int _al_ogl_get_glformat(int format, int component)
{
#if !defined ALLEGRO_CFG_OPENGLES
static int glformats[ALLEGRO_NUM_PIXEL_FORMATS][3] = {
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_BGRA}, 
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA}, 
{GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_BGRA}, 
{GL_RGB8, GL_UNSIGNED_BYTE, GL_BGR}, 
{GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB}, 
{GL_RGB5, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_BGRA}, 
{GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA}, 
{GL_RGB5_A1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_BGRA}, 
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA}, 
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA}, 
{GL_RGB8, GL_UNSIGNED_BYTE, GL_RGB}, 
{GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, GL_RGB}, 
{GL_RGB5, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_RGBA}, 
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA}, 
{GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_BGRA}, 
{GL_RGBA32F_ARB, GL_FLOAT, GL_RGBA}, 
{GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA}, 
{GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA}, 
{GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LUMINANCE}, 
{GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA}, 
{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA}, 
{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_UNSIGNED_INT_8_8_8_8, GL_RGBA}, 
};
if (al_get_opengl_version() >= _ALLEGRO_OPENGL_VERSION_3_0) {
glformats[ALLEGRO_PIXEL_FORMAT_SINGLE_CHANNEL_8][0] = GL_RED;
glformats[ALLEGRO_PIXEL_FORMAT_SINGLE_CHANNEL_8][2] = GL_RED;
}
#else
static const int glformats[ALLEGRO_NUM_PIXEL_FORMATS][3] = {
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB}, 
{0, 0, 0},
{GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA}, 
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA}, 
{GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA}, 
{GL_LUMINANCE, GL_UNSIGNED_BYTE, GL_LUMINANCE}, 
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
};
#endif
return glformats[format][component];
}
static ALLEGRO_BITMAP_INTERFACE glbmp_vt;
#define SWAP(type, x, y) {type temp = x; x = y; y = temp;}
#define ERR(e) case e: return #e;
char const *_al_gl_error_string(GLenum e)
{
switch (e) {
ERR(GL_NO_ERROR)
ERR(GL_INVALID_ENUM)
ERR(GL_INVALID_VALUE)
ERR(GL_INVALID_OPERATION)
#if defined(ALLEGRO_CFG_OPENGL_FIXED_FUNCTION)
ERR(GL_STACK_OVERFLOW)
ERR(GL_STACK_UNDERFLOW)
#endif
ERR(GL_OUT_OF_MEMORY)
#if defined(ALLEGRO_CFG_OPENGL_PROGRAMMABLE_PIPELINE)
ERR(GL_INVALID_FRAMEBUFFER_OPERATION)
#endif
}
return "UNKNOWN";
}
#undef ERR
static INLINE void transform_vertex(float* x, float* y, float* z)
{
al_transform_coordinates_3d(al_get_current_transform(), x, y, z);
}
static void draw_quad(ALLEGRO_BITMAP *bitmap,
ALLEGRO_COLOR tint,
float sx, float sy, float sw, float sh,
int flags)
{
float tex_l, tex_t, tex_r, tex_b, w, h, true_w, true_h;
float dw = sw, dh = sh;
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap = bitmap->extra;
ALLEGRO_OGL_BITMAP_VERTEX *verts;
ALLEGRO_DISPLAY *disp = al_get_current_display();
(void)flags;
if (disp->num_cache_vertices != 0 && ogl_bitmap->texture != disp->cache_texture) {
disp->vt->flush_vertex_cache(disp);
}
disp->cache_texture = ogl_bitmap->texture;
verts = disp->vt->prepare_vertex_cache(disp, 6);
tex_l = ogl_bitmap->left;
tex_r = ogl_bitmap->right;
tex_t = ogl_bitmap->top;
tex_b = ogl_bitmap->bottom;
w = bitmap->w;
h = bitmap->h;
true_w = ogl_bitmap->true_w;
true_h = ogl_bitmap->true_h;
tex_l += sx / true_w;
tex_t -= sy / true_h;
tex_r -= (w - sx - sw) / true_w;
tex_b += (h - sy - sh) / true_h;
verts[0].x = 0;
verts[0].y = dh;
verts[0].z = 0;
verts[0].tx = tex_l;
verts[0].ty = tex_b;
verts[0].r = tint.r;
verts[0].g = tint.g;
verts[0].b = tint.b;
verts[0].a = tint.a;
verts[1].x = 0;
verts[1].y = 0;
verts[1].z = 0;
verts[1].tx = tex_l;
verts[1].ty = tex_t;
verts[1].r = tint.r;
verts[1].g = tint.g;
verts[1].b = tint.b;
verts[1].a = tint.a;
verts[2].x = dw;
verts[2].y = dh;
verts[2].z = 0;
verts[2].tx = tex_r;
verts[2].ty = tex_b;
verts[2].r = tint.r;
verts[2].g = tint.g;
verts[2].b = tint.b;
verts[2].a = tint.a;
verts[4].x = dw;
verts[4].y = 0;
verts[4].z = 0;
verts[4].tx = tex_r;
verts[4].ty = tex_t;
verts[4].r = tint.r;
verts[4].g = tint.g;
verts[4].b = tint.b;
verts[4].a = tint.a;
if (disp->cache_enabled) {
transform_vertex(&verts[0].x, &verts[0].y, &verts[0].z);
transform_vertex(&verts[1].x, &verts[1].y, &verts[1].z);
transform_vertex(&verts[2].x, &verts[2].y, &verts[2].z);
transform_vertex(&verts[4].x, &verts[4].y, &verts[4].z);
}
verts[3] = verts[1];
verts[5] = verts[2];
if (!disp->cache_enabled)
disp->vt->flush_vertex_cache(disp);
}
#undef SWAP
static void ogl_draw_bitmap_region(ALLEGRO_BITMAP *bitmap,
ALLEGRO_COLOR tint, float sx, float sy,
float sw, float sh, int flags)
{
ALLEGRO_BITMAP *target = al_get_target_bitmap();
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_target;
ALLEGRO_DISPLAY *disp = _al_get_bitmap_display(target);
if (target->parent) {
target = target->parent;
}
ogl_target = target->extra;
if (!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP) && !bitmap->locked &&
!target->locked) {
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_source = bitmap->extra;
if (ogl_source->is_backbuffer) {
float xtrans, ytrans;
ASSERT(!ogl_target->is_backbuffer);
if (_al_transform_is_translation(al_get_current_transform(),
&xtrans, &ytrans)) {
if (ogl_target->fbo_info)
_al_ogl_set_target_bitmap(disp, bitmap);
if (xtrans < target->cl) {
sx -= xtrans - target->cl;
sw += xtrans - target->cl;
xtrans = target->cl;
}
if (ytrans < target->ct) {
sy -= ytrans - target->ct;
sh += ytrans - target->ct;
ytrans = target->ct;
}
if (xtrans + sw > target->cr_excl) {
sw = target->cr_excl - xtrans;
}
if (ytrans + sh > target->cb_excl) {
sh = target->cb_excl - ytrans;
}
glBindTexture(GL_TEXTURE_2D, ogl_target->texture);
glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
xtrans, target->h - ytrans - sh,
sx, bitmap->h - sy - sh,
sw, sh);
if (ogl_target->fbo_info)
_al_ogl_set_target_bitmap(disp, target);
return;
}
ASSERT(0);
}
}
if (disp->ogl_extras->opengl_target == target) {
draw_quad(bitmap, tint, sx, sy, sw, sh, flags);
return;
}
_al_draw_bitmap_region_memory(bitmap, tint, sx, sy, sw, sh, 0, 0, flags);
}
static int pot(int x)
{
int y = 1;
while (y < x) y *= 2;
return y;
}
static bool ogl_upload_bitmap(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap = bitmap->extra;
int w = bitmap->w;
int h = bitmap->h;
int bitmap_format = al_get_bitmap_format(bitmap);
int bitmap_flags = al_get_bitmap_flags(bitmap);
bool post_generate_mipmap = false;
GLenum e;
int filter;
int gl_filters[] = {
GL_NEAREST, GL_LINEAR,
GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR
};
if (ogl_bitmap->texture == 0) {
glGenTextures(1, &ogl_bitmap->texture);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glGenTextures failed: %s\n", _al_gl_error_string(e));
}
else {
ALLEGRO_DEBUG("Created new OpenGL texture %d (%dx%d, format %s)\n",
ogl_bitmap->texture,
ogl_bitmap->true_w, ogl_bitmap->true_h,
_al_pixel_format_name(bitmap_format));
}
}
glBindTexture(GL_TEXTURE_2D, ogl_bitmap->texture);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glBindTexture for texture %d failed (%s).\n",
ogl_bitmap->texture, _al_gl_error_string(e));
}
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
filter = (bitmap_flags & ALLEGRO_MIPMAP) ? 2 : 0;
if (bitmap_flags & ALLEGRO_MIN_LINEAR) {
filter++;
}
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filters[filter]);
filter = 0;
if (bitmap_flags & ALLEGRO_MAG_LINEAR) {
filter++;
}
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filters[filter]);
#if 0
if (al_get_opengl_extension_list()->ALLEGRO_GL_EXT_texture_filter_anisotropic) {
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}
#endif
if (bitmap_flags & ALLEGRO_MIPMAP) {
if (al_get_opengl_extension_list()->ALLEGRO_GL_EXT_framebuffer_object ||
al_get_opengl_extension_list()->ALLEGRO_GL_OES_framebuffer_object ||
IS_OPENGLES ) {
post_generate_mipmap = true;
}
else {
#if defined(ALLEGRO_CFG_OPENGL_FIXED_FUNCTION)
glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glTexParameteri for texture %d failed (%s).\n",
ogl_bitmap->texture, _al_gl_error_string(e));
}
#endif
}
}
if (!IS_OPENGLES) {
if (ogl_bitmap->true_w != bitmap->w ||
ogl_bitmap->true_h != bitmap->h ||
bitmap_format == ALLEGRO_PIXEL_FORMAT_ABGR_F32) {
unsigned char *buf;
buf = al_calloc(ogl_bitmap->true_h, ogl_bitmap->true_w);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
glTexImage2D(GL_TEXTURE_2D, 0, get_glformat(bitmap_format, 0),
ogl_bitmap->true_w, ogl_bitmap->true_h, 0,
GL_ALPHA, GL_UNSIGNED_BYTE, buf);
e = glGetError();
al_free(buf);
}
else {
glTexImage2D(GL_TEXTURE_2D, 0, get_glformat(bitmap_format, 0),
ogl_bitmap->true_w, ogl_bitmap->true_h, 0,
get_glformat(bitmap_format, 2), get_glformat(bitmap_format, 1),
NULL);
e = glGetError();
}
}
else {
unsigned char *buf;
int pix_size = al_get_pixel_size(bitmap_format);
buf = al_calloc(pix_size,
ogl_bitmap->true_h * ogl_bitmap->true_w);
glPixelStorei(GL_UNPACK_ALIGNMENT, pix_size);
glTexImage2D(GL_TEXTURE_2D, 0, get_glformat(bitmap_format, 0),
ogl_bitmap->true_w, ogl_bitmap->true_h, 0,
get_glformat(bitmap_format, 2),
get_glformat(bitmap_format, 1), buf);
e = glGetError();
al_free(buf);
}
if (e) {
ALLEGRO_ERROR("glTexImage2D for format %s, size %dx%d failed (%s)\n",
_al_pixel_format_name(bitmap_format),
ogl_bitmap->true_w, ogl_bitmap->true_h,
_al_gl_error_string(e));
glDeleteTextures(1, &ogl_bitmap->texture);
ogl_bitmap->texture = 0;
return false;
}
if (post_generate_mipmap) {
glGenerateMipmapEXT(GL_TEXTURE_2D);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glGenerateMipmapEXT for texture %d failed (%s).\n",
ogl_bitmap->texture, _al_gl_error_string(e));
}
}
ogl_bitmap->left = 0;
ogl_bitmap->right = (float) w / ogl_bitmap->true_w;
ogl_bitmap->top = (float) h / ogl_bitmap->true_h;
ogl_bitmap->bottom = 0;
return true;
}
static void ogl_update_clipping_rectangle(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_DISPLAY *ogl_disp = al_get_current_display();
ALLEGRO_BITMAP *target_bitmap = bitmap;
if (bitmap->parent) {
target_bitmap = bitmap->parent;
}
if (ogl_disp->ogl_extras->opengl_target == target_bitmap) {
_al_ogl_setup_bitmap_clipping(bitmap);
}
}
static void ogl_destroy_bitmap(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap = bitmap->extra;
ALLEGRO_DISPLAY *disp;
ALLEGRO_DISPLAY *bmp_disp;
ALLEGRO_DISPLAY *old_disp = NULL;
ASSERT(!al_is_sub_bitmap(bitmap));
bmp_disp = _al_get_bitmap_display(bitmap);
disp = al_get_current_display();
if (bmp_disp->ogl_extras->is_shared == false &&
bmp_disp != disp) {
old_disp = disp;
_al_set_current_display_only(bmp_disp);
}
if (bmp_disp->ogl_extras->opengl_target == bitmap) {
bmp_disp->ogl_extras->opengl_target = NULL;
}
al_remove_opengl_fbo(bitmap);
if (ogl_bitmap->texture) {
glDeleteTextures(1, &ogl_bitmap->texture);
ogl_bitmap->texture = 0;
}
if (old_disp) {
_al_set_current_display_only(old_disp);
}
al_free(ogl_bitmap);
}
static void ogl_bitmap_pointer_changed(ALLEGRO_BITMAP *bitmap,
ALLEGRO_BITMAP *old)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *extra = bitmap->extra;
if (extra && extra->fbo_info) {
ASSERT(extra->fbo_info->owner == old);
extra->fbo_info->owner = bitmap;
}
}
static bool can_flip_blocks(ALLEGRO_PIXEL_FORMAT format)
{
switch (format) {
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT1:
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT3:
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT5:
return true;
default:
return false;
}
}
static void ogl_flip_blocks(ALLEGRO_LOCKED_REGION *lr, int wc, int hc)
{
#define SWAP(x, y) do { unsigned char t = x; x = y; y = t; } while (0)
int x, y;
unsigned char* data = lr->data;
ASSERT(can_flip_blocks(lr->format));
switch (lr->format) {
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT1: {
for (y = 0; y < hc; y++) {
unsigned char* row = data;
for (x = 0; x < wc; x++) {
row += 4;
SWAP(row[0], row[3]);
SWAP(row[2], row[1]);
row += 4;
}
data += lr->pitch;
}
break;
}
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT3: {
for (y = 0; y < hc; y++) {
unsigned char* row = data;
for (x = 0; x < wc; x++) {
SWAP(row[0], row[6]);
SWAP(row[1], row[7]);
SWAP(row[2], row[4]);
SWAP(row[3], row[5]);
row += 8;
row += 4;
SWAP(row[0], row[3]);
SWAP(row[2], row[1]);
row += 4;
}
data += lr->pitch;
}
break;
}
case ALLEGRO_PIXEL_FORMAT_COMPRESSED_RGBA_DXT5: {
for (y = 0; y < hc; y++) {
unsigned char* row = data;
for (x = 0; x < wc; x++) {
uint16_t bit_row0, bit_row1, bit_row2, bit_row3;
row += 2;
bit_row0 = (((uint16_t)row[0]) | (uint16_t)row[1] << 8) << 4;
bit_row1 = (((uint16_t)row[1]) | (uint16_t)row[2] << 8) >> 4;
bit_row2 = (((uint16_t)row[3]) | (uint16_t)row[4] << 8) << 4;
bit_row3 = (((uint16_t)row[4]) | (uint16_t)row[5] << 8) >> 4;
row[0] = (unsigned char)(bit_row3 & 0x00ff);
row[1] = (unsigned char)((bit_row2 & 0x00ff) | ((bit_row3 & 0xff00) >> 8));
row[2] = (unsigned char)((bit_row2 & 0xff00) >> 8);
row[3] = (unsigned char)(bit_row1 & 0x00ff);
row[4] = (unsigned char)((bit_row0 & 0x00ff) | ((bit_row1 & 0xff00) >> 8));
row[5] = (unsigned char)((bit_row0 & 0xff00) >> 8);
row += 6;
row += 4;
SWAP(row[0], row[3]);
SWAP(row[2], row[1]);
row += 4;
}
data += lr->pitch;
}
break;
}
default:
(void)x;
(void)y;
(void)data;
(void)wc;
(void)hc;
}
#undef SWAP
}
static ALLEGRO_LOCKED_REGION *ogl_lock_compressed_region(ALLEGRO_BITMAP *bitmap,
int x, int y, int w, int h, int flags)
{
#if !defined ALLEGRO_CFG_OPENGLES
ALLEGRO_BITMAP_EXTRA_OPENGL *const ogl_bitmap = bitmap->extra;
ALLEGRO_DISPLAY *disp;
ALLEGRO_DISPLAY *old_disp = NULL;
GLenum e;
bool ok = true;
int bitmap_format = al_get_bitmap_format(bitmap);
int block_width = al_get_pixel_block_width(bitmap_format);
int block_height = al_get_pixel_block_height(bitmap_format);
int block_size = al_get_pixel_block_size(bitmap_format);
int xc = x / block_width;
int yc = y / block_width;
int wc = w / block_width;
int hc = h / block_width;
int true_wc = ogl_bitmap->true_w / block_width;
int true_hc = ogl_bitmap->true_h / block_height;
int gl_yc = _al_get_least_multiple(bitmap->h, block_height) / block_height - yc - hc;
if (!can_flip_blocks(bitmap_format)) {
return NULL;
}
if (flags & ALLEGRO_LOCK_WRITEONLY) {
int pitch = wc * block_size;
ogl_bitmap->lock_buffer = al_malloc(pitch * hc);
if (ogl_bitmap->lock_buffer == NULL) {
return NULL;
}
bitmap->locked_region.data = ogl_bitmap->lock_buffer + pitch * (hc - 1);
bitmap->locked_region.format = bitmap_format;
bitmap->locked_region.pitch = -pitch;
bitmap->locked_region.pixel_size = block_size;
return &bitmap->locked_region;
}
disp = al_get_current_display();
if (!disp ||
(_al_get_bitmap_display(bitmap)->ogl_extras->is_shared == false &&
_al_get_bitmap_display(bitmap) != disp))
{
old_disp = disp;
_al_set_current_display_only(_al_get_bitmap_display(bitmap));
}
#if defined(GL_CLIENT_PIXEL_STORE_BIT)
glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
#endif
{
glPixelStorei(GL_PACK_ALIGNMENT, 1);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glPixelStorei(GL_PACK_ALIGNMENT, %d) failed (%s).\n",
1, _al_gl_error_string(e));
ok = false;
}
}
if (ok) {
ogl_bitmap->lock_buffer = al_malloc(true_wc * true_hc * block_size);
if (ogl_bitmap->lock_buffer != NULL) {
glBindTexture(GL_TEXTURE_2D, ogl_bitmap->texture);
glGetCompressedTexImage(GL_TEXTURE_2D, 0, ogl_bitmap->lock_buffer);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glGetCompressedTexImage for format %s failed (%s).\n",
_al_pixel_format_name(bitmap_format), _al_gl_error_string(e));
al_free(ogl_bitmap->lock_buffer);
ogl_bitmap->lock_buffer = NULL;
ok = false;
}
else {
if (flags == ALLEGRO_LOCK_READWRITE) {
int y;
int src_pitch = true_wc * block_size;
int dest_pitch = wc * block_size;
char* dest_ptr = (char*)ogl_bitmap->lock_buffer;
char* src_ptr = (char*)ogl_bitmap->lock_buffer +
src_pitch * gl_yc + block_size * xc;
for (y = 0; y < hc; y++) {
memmove(dest_ptr, src_ptr, dest_pitch);
src_ptr += src_pitch;
dest_ptr += dest_pitch;
}
bitmap->locked_region.data = ogl_bitmap->lock_buffer +
dest_pitch * (hc - 1);
bitmap->locked_region.pitch = -dest_pitch;
}
else {
int pitch = true_wc * block_size;
bitmap->locked_region.data = ogl_bitmap->lock_buffer +
pitch * (gl_yc + hc - 1) + block_size * xc;
bitmap->locked_region.pitch = -pitch;
}
bitmap->locked_region.format = bitmap_format;
bitmap->locked_region.pixel_size = block_size;
}
}
else {
ok = false;
}
}
#if defined(GL_CLIENT_PIXEL_STORE_BIT)
glPopClientAttrib();
#endif
if (old_disp != NULL) {
_al_set_current_display_only(old_disp);
}
if (ok) {
ogl_flip_blocks(&bitmap->locked_region, wc, hc);
return &bitmap->locked_region;
}
ALLEGRO_ERROR("Failed to lock region\n");
ASSERT(ogl_bitmap->lock_buffer == NULL);
return NULL;
#else
(void)bitmap;
(void)x;
(void)y;
(void)w;
(void)h;
(void)flags;
return NULL;
#endif
}
static void ogl_unlock_compressed_region(ALLEGRO_BITMAP *bitmap)
{
#if !defined ALLEGRO_CFG_OPENGLES
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap = bitmap->extra;
int lock_format = bitmap->locked_region.format;
ALLEGRO_DISPLAY *old_disp = NULL;
ALLEGRO_DISPLAY *disp;
GLenum e;
int block_size = al_get_pixel_block_size(lock_format);
int block_width = al_get_pixel_block_width(lock_format);
int block_height = al_get_pixel_block_height(lock_format);
int data_size = bitmap->lock_h * bitmap->lock_w /
(block_width * block_height) * block_size;
int gl_y = _al_get_least_multiple(bitmap->h, block_height) - bitmap->lock_y - bitmap->lock_h;
ASSERT(can_flip_blocks(bitmap->locked_region.format));
if ((bitmap->lock_flags & ALLEGRO_LOCK_READONLY)) {
goto EXIT;
}
ogl_flip_blocks(&bitmap->locked_region, bitmap->lock_w / block_width,
bitmap->lock_h / block_height);
disp = al_get_current_display();
if (!disp ||
(_al_get_bitmap_display(bitmap)->ogl_extras->is_shared == false &&
_al_get_bitmap_display(bitmap) != disp))
{
old_disp = disp;
_al_set_current_display_only(_al_get_bitmap_display(bitmap));
}
#if defined(GL_CLIENT_PIXEL_STORE_BIT)
glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
#endif
{
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glPixelStorei(GL_UNPACK_ALIGNMENT, %d) failed (%s).\n",
1, _al_gl_error_string(e));
}
}
glBindTexture(GL_TEXTURE_2D, ogl_bitmap->texture);
glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
bitmap->lock_x, gl_y,
bitmap->lock_w, bitmap->lock_h,
get_glformat(lock_format, 0),
data_size,
ogl_bitmap->lock_buffer);
e = glGetError();
if (e) {
ALLEGRO_ERROR("glCompressedTexSubImage2D for format %s failed (%s).\n",
_al_pixel_format_name(lock_format), _al_gl_error_string(e));
}
#if defined(GL_CLIENT_PIXEL_STORE_BIT)
glPopClientAttrib();
#endif
if (old_disp) {
_al_set_current_display_only(old_disp);
}
EXIT:
al_free(ogl_bitmap->lock_buffer);
ogl_bitmap->lock_buffer = NULL;
#else
(void)bitmap;
#endif
}
static void ogl_backup_dirty_bitmap(ALLEGRO_BITMAP *b)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap = b->extra;
ALLEGRO_LOCKED_REGION *lr;
int bitmap_flags = al_get_bitmap_flags(b);
if (b->parent)
return;
if ((bitmap_flags & ALLEGRO_MEMORY_BITMAP) ||
(bitmap_flags & ALLEGRO_NO_PRESERVE_TEXTURE) ||
!b->dirty ||
ogl_bitmap->is_backbuffer)
return;
ALLEGRO_DEBUG("Backing up dirty bitmap %p\n", b);
lr = al_lock_bitmap(
b,
_al_get_bitmap_memory_format(b),
ALLEGRO_LOCK_READONLY
);
if (lr) {
int line_size = al_get_pixel_size(lr->format) * b->w;
int y;
for (y = 0; y < b->h; y++) {
unsigned char *p = ((unsigned char *)lr->data) + lr->pitch * y;
unsigned char *p2;
p2 = ((unsigned char *)b->memory) + line_size * (b->h-1-y);
memcpy(p2, p, line_size);
}
al_unlock_bitmap(b);
b->dirty = false;
}
else {
ALLEGRO_WARN("Failed to lock dirty bitmap %p\n", b);
}
}
static ALLEGRO_BITMAP_INTERFACE *ogl_bitmap_driver(void)
{
if (glbmp_vt.draw_bitmap_region) {
return &glbmp_vt;
}
glbmp_vt.draw_bitmap_region = ogl_draw_bitmap_region;
glbmp_vt.upload_bitmap = ogl_upload_bitmap;
glbmp_vt.update_clipping_rectangle = ogl_update_clipping_rectangle;
glbmp_vt.destroy_bitmap = ogl_destroy_bitmap;
glbmp_vt.bitmap_pointer_changed = ogl_bitmap_pointer_changed;
#if defined(ALLEGRO_CFG_OPENGLES)
glbmp_vt.lock_region = _al_ogl_lock_region_gles;
glbmp_vt.unlock_region = _al_ogl_unlock_region_gles;
#else
glbmp_vt.lock_region = _al_ogl_lock_region_new;
glbmp_vt.unlock_region = _al_ogl_unlock_region_new;
#endif
glbmp_vt.lock_compressed_region = ogl_lock_compressed_region;
glbmp_vt.unlock_compressed_region = ogl_unlock_compressed_region;
glbmp_vt.backup_dirty_bitmap = ogl_backup_dirty_bitmap;
return &glbmp_vt;
}
ALLEGRO_BITMAP *_al_ogl_create_bitmap(ALLEGRO_DISPLAY *d, int w, int h,
int format, int flags)
{
ALLEGRO_BITMAP *bitmap;
ALLEGRO_BITMAP_EXTRA_OPENGL *extra;
int true_w;
int true_h;
int block_width;
int block_height;
(void)d;
format = _al_get_real_pixel_format(d, format);
ASSERT(_al_pixel_format_is_real(format));
block_width = al_get_pixel_block_width(format);
block_height = al_get_pixel_block_width(format);
true_w = _al_get_least_multiple(w, block_width);
true_h = _al_get_least_multiple(h, block_height);
if (_al_pixel_format_is_compressed(format)) {
if (!al_get_opengl_extension_list()->ALLEGRO_GL_EXT_texture_compression_s3tc) {
ALLEGRO_DEBUG("Device does not support S3TC compressed textures.\n");
return NULL;
}
}
if (!d->extra_settings.settings[ALLEGRO_SUPPORT_NPOT_BITMAP]) {
true_w = pot(true_w);
true_h = pot(true_h);
}
if (true_w < 16) true_w = 16;
if (true_h < 16) true_h = 16;
if (IS_ANDROID) {
int mod = true_w % 32;
if (mod != 0) {
true_w += 32 - mod;
}
}
ASSERT(true_w % block_width == 0);
ASSERT(true_h % block_height == 0);
bitmap = al_calloc(1, sizeof *bitmap);
ASSERT(bitmap);
bitmap->extra = al_calloc(1, sizeof(ALLEGRO_BITMAP_EXTRA_OPENGL));
ASSERT(bitmap->extra);
extra = bitmap->extra;
bitmap->vt = ogl_bitmap_driver();
bitmap->_memory_format =
_al_pixel_format_is_compressed(format) ? ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE : format;
bitmap->pitch = true_w * al_get_pixel_size(bitmap->_memory_format);
bitmap->_format = format;
bitmap->_flags = flags | _ALLEGRO_INTERNAL_OPENGL;
extra->true_w = true_w;
extra->true_h = true_h;
if (!(flags & ALLEGRO_NO_PRESERVE_TEXTURE)) {
bitmap->memory = al_calloc(1, al_get_pixel_size(bitmap->_memory_format)*w*h);
}
return bitmap;
}
void _al_ogl_upload_bitmap_memory(ALLEGRO_BITMAP *bitmap, int format, void *ptr)
{
int w = bitmap->w;
int h = bitmap->h;
int pixsize = al_get_pixel_size(format);
int y;
ALLEGRO_BITMAP *tmp;
ALLEGRO_LOCKED_REGION *lr;
uint8_t *dst;
uint8_t *src;
ASSERT(ptr);
ASSERT(al_get_current_display() == _al_get_bitmap_display(bitmap));
tmp = _al_create_bitmap_params(_al_get_bitmap_display(bitmap), w, h, format,
al_get_bitmap_flags(bitmap), 0, 0);
ASSERT(tmp);
lr = al_lock_bitmap(tmp, format, ALLEGRO_LOCK_WRITEONLY);
ASSERT(lr);
dst = (uint8_t *)lr->data;
src = ((uint8_t *)ptr) + (pixsize * w * (h-1));
for (y = 0; y < h; y++) {
memcpy(dst, src, pixsize * w);
dst += lr->pitch;
src -= pixsize * w; 
}
al_unlock_bitmap(tmp);
((ALLEGRO_BITMAP_EXTRA_OPENGL *)bitmap->extra)->texture =
((ALLEGRO_BITMAP_EXTRA_OPENGL *)tmp->extra)->texture;
((ALLEGRO_BITMAP_EXTRA_OPENGL *)tmp->extra)->texture = 0;
al_destroy_bitmap(tmp);
}
GLuint al_get_opengl_texture(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *extra;
if (bitmap->parent)
bitmap = bitmap->parent;
if (!(al_get_bitmap_flags(bitmap) & _ALLEGRO_INTERNAL_OPENGL))
return 0;
extra = bitmap->extra;
return extra->texture;
}
void al_remove_opengl_fbo(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap;
if (bitmap->parent)
bitmap = bitmap->parent;
if (!(al_get_bitmap_flags(bitmap) & _ALLEGRO_INTERNAL_OPENGL))
return;
ogl_bitmap = bitmap->extra;
if (!ogl_bitmap->fbo_info)
return;
ASSERT(ogl_bitmap->fbo_info->fbo_state > FBO_INFO_UNUSED);
ASSERT(ogl_bitmap->fbo_info->fbo != 0);
ALLEGRO_FBO_INFO *info = ogl_bitmap->fbo_info;
_al_ogl_del_fbo(info);
if (info->fbo_state == FBO_INFO_PERSISTENT) {
al_free(info);
}
else {
_al_ogl_reset_fbo_info(info);
}
}
GLuint al_get_opengl_fbo(ALLEGRO_BITMAP *bitmap)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap;
if (bitmap->parent)
bitmap = bitmap->parent;
if (!(al_get_bitmap_flags(bitmap) & _ALLEGRO_INTERNAL_OPENGL))
return 0;
ogl_bitmap = bitmap->extra;
if (!ogl_bitmap->fbo_info) {
if (!_al_ogl_create_persistent_fbo(bitmap)) {
return 0;
}
}
if (ogl_bitmap->fbo_info->fbo_state == FBO_INFO_TRANSIENT) {
ogl_bitmap->fbo_info = _al_ogl_persist_fbo(_al_get_bitmap_display(bitmap),
ogl_bitmap->fbo_info);
}
return ogl_bitmap->fbo_info->fbo;
}
bool al_get_opengl_texture_size(ALLEGRO_BITMAP *bitmap, int *w, int *h)
{
ALLEGRO_BITMAP_EXTRA_OPENGL *ogl_bitmap;
if (bitmap->parent)
bitmap = bitmap->parent;
if (!(al_get_bitmap_flags(bitmap) & _ALLEGRO_INTERNAL_OPENGL)) {
*w = 0;
*h = 0;
return false;
}
ogl_bitmap = bitmap->extra;
*w = ogl_bitmap->true_w;
*h = ogl_bitmap->true_h;
return true;
}
void al_get_opengl_texture_position(ALLEGRO_BITMAP *bitmap, int *u, int *v)
{
ASSERT(bitmap);
ASSERT(u);
ASSERT(v);
*u = bitmap->xofs;
*v = bitmap->yofs;
}
