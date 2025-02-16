#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_opengl.h"
#include "allegro5/opengl/gl_ext.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_opengl.h"
#include "allegro5/internal/aintern_display.h"
#include "allegro5/internal/aintern_system.h"
#if defined ALLEGRO_WINDOWS
#include "../win/wgl.h"
#elif defined ALLEGRO_UNIX && !defined ALLEGRO_EXCLUDE_GLX
#include "allegro5/internal/aintern_xdisplay.h"
#include "allegro5/internal/aintern_xsystem.h"
#endif
#if defined ALLEGRO_MACOSX
#include <OpenGL/glu.h>
#elif !defined ALLEGRO_CFG_OPENGLES
#include <GL/glu.h>
#endif
ALLEGRO_DEBUG_CHANNEL("opengl")
#if defined(ALLEGRO_HAVE_DYNAMIC_LINK)
#include <dlfcn.h>
static void *__libgl_handle = NULL;
typedef void *(*GLXGETPROCADDRESSARBPROC) (const GLubyte *);
static GLXGETPROCADDRESSARBPROC alXGetProcAddress;
#else 
#if defined ALLEGRO_GLXGETPROCADDRESSARB
#define alXGetProcAddress glXGetProcAddressARB
#elif defined ALLEGRO_RASPBERRYPI
#define alXGetProcAddress eglGetProcAddress
#else
#define alXGetProcAddress glXGetProcAddress
#endif
#endif 
#if defined(ALLEGRO_MACOSX)
#include <CoreFoundation/CoreFoundation.h>
static CFBundleRef opengl_bundle_ref;
#endif
#define AGL_API(type, name, args) _ALLEGRO_gl##name##_t _al_gl##name = NULL;
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) _ALLEGRO_wgl##name##_t _al_wgl##name = NULL;
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) _ALLEGRO_glX##name##_t _al_glX##name = NULL;
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#endif
static uint32_t parse_opengl_version(const char *s)
{
char *p = (char *) s;
int v[4] = {0, 0, 0, 0};
int n;
uint32_t ver;
while (*p && !isdigit(*p)) {
p++;
}
for (n = 0; n < 4; n++) {
char *end;
long l;
errno = 0;
l = strtol(p, &end, 10);
if (errno)
break;
v[n] = _ALLEGRO_CLAMP(0, l, 255);
if (*end != '.')
break;
p = end + 1; 
}
ver = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
ALLEGRO_DEBUG("Parsed '%s' as 0x%08x\n", s, ver);
return ver;
}
static uint32_t _al_ogl_version(void)
{
char const *value = al_get_config_value(al_get_system_config(), "opengl",
"force_opengl_version");
if (value) {
uint32_t v = parse_opengl_version(value);
ALLEGRO_INFO("OpenGL version forced to %d.%d.%d.%d.\n",
(v >> 24) & 0xff,
(v >> 16) & 0xff,
(v >> 8) & 0xff,
(v & 0xff));
return v;
}
const char *str;
str = (const char *)glGetString(GL_VERSION);
if (str) {
#if defined(ALLEGRO_CFG_OPENGLES)
char *str2 = strstr(str, "ES ");
if (str2)
str = str2 + 3;
#endif
return parse_opengl_version(str);
}
else {
return _ALLEGRO_OPENGL_VERSION_1_0;
}
}
static void print_extensions(char const *extension)
{
char buf[80];
char *start;
ASSERT(extension);
while (*extension != '\0') {
start = buf;
_al_sane_strncpy(buf, extension, 80);
while ((*start != ' ') && (*start != '\0')) {
extension++;
start++;
}
*start = '\0';
if (*extension != '\0')
extension++;
ALLEGRO_DEBUG("%s\n", buf);
}
}
#if !defined ALLEGRO_CFG_OPENGLES
static void print_extensions_3_0(void)
{
int i;
GLint n;
GLubyte const *name;
glGetIntegerv(GL_NUM_EXTENSIONS, &n);
for (i = 0; i < n; i++) {
name = glGetStringi(GL_EXTENSIONS, i);
ALLEGRO_DEBUG("%s\n", name);
}
}
#endif
uint32_t al_get_opengl_version(void)
{
ALLEGRO_DISPLAY *ogl_disp;
ogl_disp = al_get_current_display();
if (!ogl_disp || !ogl_disp->ogl_extras)
return 0x0;
return ogl_disp->ogl_extras->ogl_info.version;
}
int al_get_opengl_variant(void)
{
#if defined ALLEGRO_CFG_OPENGLES
return ALLEGRO_OPENGL_ES;
#else
return ALLEGRO_DESKTOP_OPENGL;
#endif
}
static ALLEGRO_OGL_EXT_LIST *create_extension_list(void)
{
ALLEGRO_OGL_EXT_LIST *ret = al_calloc(1, sizeof(ALLEGRO_OGL_EXT_LIST));
if (!ret) {
return NULL;
}
return ret;
}
static ALLEGRO_OGL_EXT_API *create_extension_api_table(void)
{
ALLEGRO_OGL_EXT_API *ret = al_calloc(1, sizeof(ALLEGRO_OGL_EXT_API));
if (!ret) {
return NULL;
}
return ret;
}
typedef void (*VOID_FPTR)(void);
static VOID_FPTR load_extension(const char* name)
{
VOID_FPTR fptr = NULL;
#if defined(ALLEGRO_WINDOWS)
fptr = (VOID_FPTR)wglGetProcAddress(name);
#elif defined ALLEGRO_UNIX
fptr = (VOID_FPTR)alXGetProcAddress((const GLubyte*)name);
#elif defined ALLEGRO_MACOSX
CFStringRef cfstr = CFStringCreateWithCStringNoCopy(NULL, name,
kCFStringEncodingUTF8, kCFAllocatorNull);
if (cfstr) {
fptr = (VOID_FPTR)CFBundleGetFunctionPointerForName(opengl_bundle_ref, cfstr);
CFRelease(cfstr);
}
#elif defined ALLEGRO_SDL
fptr = SDL_GL_GetProcAddress(name);
#endif
if (fptr) {
ALLEGRO_DEBUG("%s successfully loaded (%p)\n", name, fptr);
}
return fptr;
}
static void load_extensions(ALLEGRO_OGL_EXT_API *ext)
{
if (!ext) {
return;
}
#if defined(ALLEGRO_UNIX)
#if defined(ALLEGRO_HAVE_DYNAMIC_LINK)
if (!alXGetProcAddress) {
return;
}
#endif
#endif
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_gl##name##_t)load_extension("gl" #name);
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_wgl##name##_t)load_extension("wgl" #name);
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_gl##name##_t)load_extension("gl" #name);
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_glX##name##_t)load_extension("glX" #name);
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_MACOSX
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_gl##name##_t)load_extension("gl" #name);
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_SDL
#define AGL_API(type, name, args) ext->name = (_ALLEGRO_gl##name##_t)load_extension("gl" #name);
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#endif
}
void _al_ogl_set_extensions(ALLEGRO_OGL_EXT_API *ext)
{
if (!ext) {
return;
}
#define AGL_API(type, name, args) _al_gl##name = ext->name;
#include "allegro5/opengl/GLext/gl_ext_api.h"
#undef AGL_API
#if defined(ALLEGRO_WINDOWS)
#define AGL_API(type, name, args) _al_wgl##name = ext->name;
#include "allegro5/opengl/GLext/wgl_ext_api.h"
#undef AGL_API
#elif defined ALLEGRO_UNIX
#define AGL_API(type, name, args) _al_glX##name = ext->name;
#include "allegro5/opengl/GLext/glx_ext_api.h"
#undef AGL_API
#endif
}
static void destroy_extension_api_table(ALLEGRO_OGL_EXT_API *ext)
{
if (ext) {
al_free(ext);
}
}
static void destroy_extension_list(ALLEGRO_OGL_EXT_LIST *list)
{
if (list) {
al_free(list);
}
}
int _al_ogl_look_for_an_extension(const char *name, const GLubyte *extensions)
{
const GLubyte *start;
GLubyte *where, *terminator;
ASSERT(extensions);
where = (GLubyte *) strchr(name, ' ');
if (where || *name == '\0')
return false;
start = extensions;
for (;;) {
where = (GLubyte *) strstr((const char *)start, name);
if (!where)
break;
terminator = where + strlen(name);
if (where == start || *(where - 1) == ' ')
if (*terminator == ' ' || *terminator == '\0')
return true;
start = terminator;
}
return false;
}
static bool _ogl_is_extension_supported(const char *extension,
ALLEGRO_DISPLAY *disp)
{
int ret = 0;
GLubyte const *ext_str;
#if !defined ALLEGRO_CFG_OPENGLES
int v = al_get_opengl_version();
#endif
(void)disp;
#if !defined ALLEGRO_CFG_OPENGLES
if (disp->flags & ALLEGRO_OPENGL_3_0 || v >= _ALLEGRO_OPENGL_VERSION_3_0) {
int i;
GLint ext_cnt;
glGetIntegerv(GL_NUM_EXTENSIONS, &ext_cnt);
for (i = 0; i < ext_cnt; i++) {
ext_str = glGetStringi(GL_EXTENSIONS, i);
if (ext_str && !strcmp(extension, (char*)ext_str)) {
ret = 1;
break;
}
}
}
else
#endif
{
ext_str = glGetString(GL_EXTENSIONS);
if (!ext_str)
return false;
ret = _al_ogl_look_for_an_extension(extension, ext_str);
}
#if defined(ALLEGRO_WINDOWS)
if (!ret && strncmp(extension, "WGL", 3) == 0) {
ALLEGRO_DISPLAY_WGL *wgl_disp = (void*)disp;
_ALLEGRO_wglGetExtensionsStringARB_t _wglGetExtensionsStringARB;
if (!wgl_disp->dc)
return false;
_wglGetExtensionsStringARB = (void *)
wglGetProcAddress("wglGetExtensionsStringARB");
if (_wglGetExtensionsStringARB) {
ret = _al_ogl_look_for_an_extension(extension, (const GLubyte *)
_wglGetExtensionsStringARB(wgl_disp->dc));
}
}
#elif defined ALLEGRO_UNIX && !defined ALLEGRO_EXCLUDE_GLX
if (!ret && strncmp(extension, "GLX", 3) == 0) {
ALLEGRO_SYSTEM_XGLX *sys = (void*)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx_disp = (void*)disp;
char const *ext;
if (!sys->gfxdisplay)
return false;
ext = glXQueryExtensionsString(sys->gfxdisplay, glx_disp->xscreen);
if (!ext) {
ext = "";
}
ret = _al_ogl_look_for_an_extension(extension, (const GLubyte *)ext);
}
#endif
return ret;
}
static bool _ogl_is_extension_with_version_supported(
const char *extension, ALLEGRO_DISPLAY *disp, uint32_t ver)
{
char const *value;
value = al_get_config_value(al_get_system_config(),
"opengl_disabled_extensions", extension);
if (value) {
ALLEGRO_WARN("%s found in [opengl_disabled_extensions].\n",
extension);
return false;
}
if (ver > 0 && disp->ogl_extras->ogl_info.version >= ver) {
return true;
}
return _ogl_is_extension_supported(extension, disp);
}
bool al_have_opengl_extension(const char *extension)
{
ALLEGRO_DISPLAY *disp;
disp = al_get_current_display();
if (!disp)
return false;
if (!(disp->flags & ALLEGRO_OPENGL))
return false;
return _ogl_is_extension_supported(extension, disp);
}
void *al_get_opengl_proc_address(const char *name)
{
void *symbol = NULL;
#if defined(ALLEGRO_MACOSX)
CFStringRef function;
#endif
ALLEGRO_DISPLAY *disp;
disp = al_get_current_display();
if (!disp)
return NULL;
if (!(disp->flags & ALLEGRO_OPENGL))
return NULL;
#if defined ALLEGRO_WINDOWS
{
ALLEGRO_DISPLAY_WGL *wgl_disp = (void*)disp;
if (!wgl_disp->dc)
return NULL;
symbol = wglGetProcAddress(name);
}
#elif defined ALLEGRO_UNIX
#if defined ALLEGRO_HAVE_DYNAMIC_LINK
if (alXGetProcAddress)
#endif
{
#if defined ALLEGRO_RASPBERRYPI
symbol = alXGetProcAddress(name);
#else
symbol = alXGetProcAddress((const GLubyte *)name);
#endif
}
#elif defined ALLEGRO_HAVE_DYNAMIC_LINK
else {
if (__al_handle) {
symbol = dlsym(__al_handle, name);
}
}
#elif defined ALLEGRO_MACOSX
function = CFStringCreateWithCString(kCFAllocatorDefault, name,
kCFStringEncodingASCII);
if (function) {
symbol =
CFBundleGetFunctionPointerForName(opengl_bundle_ref, function);
CFRelease(function);
}
#endif
if (!symbol) {
#if defined ALLEGRO_HAVE_DYNAMIC_LINK
if (!alXGetProcAddress) {
ALLEGRO_WARN("get_proc_address: libdl::dlsym: %s\n", dlerror());
}
#endif
ALLEGRO_WARN("get_proc_address : Unable to load symbol %s\n", name);
}
else {
ALLEGRO_DEBUG("get_proc_address : Symbol %s successfully loaded\n", name);
}
return symbol;
}
static void fill_in_info_struct(const GLubyte *rendereru, OPENGL_INFO *info)
{
const char *renderer = (const char *)rendereru;
ASSERT(renderer);
if (strstr(renderer, "3Dfx/Voodoo")) {
info->is_voodoo = 1;
}
else if (strstr(renderer, "Matrox G200")) {
info->is_matrox_g200 = 1;
}
else if (strstr(renderer, "RagePRO")) {
info->is_ati_rage_pro = 1;
}
else if (strstr(renderer, "RADEON 7000")) {
info->is_ati_radeon_7000 = 1;
}
else if (strstr(renderer, "Mesa DRI R200")) {
info->is_ati_r200_chip = 1;
}
else if (strstr(renderer, "Intel HD Graphics 3000 OpenGL Engine")) {
info->is_intel_hd_graphics_3000 = 1;
}
if ((strncmp(renderer, "3Dfx/Voodoo3 ", 13) == 0)
|| (strncmp(renderer, "3Dfx/Voodoo2 ", 13) == 0)
|| (strncmp(renderer, "3Dfx/Voodoo ", 12) == 0)) {
info->is_voodoo3_and_under = 1;
}
info->version = _al_ogl_version();
ALLEGRO_INFO("Assumed OpenGL version: %d.%d.%d.%d\n",
(info->version >> 24) & 0xff,
(info->version >> 16) & 0xff,
(info->version >> 8) & 0xff,
(info->version ) & 0xff);
return;
}
void _al_ogl_manage_extensions(ALLEGRO_DISPLAY *gl_disp)
{
#if defined ALLEGRO_MACOSX
CFURLRef bundle_url;
#endif
ALLEGRO_OGL_EXT_API *ext_api;
ALLEGRO_OGL_EXT_LIST *ext_list;
fill_in_info_struct(glGetString(GL_RENDERER), &(gl_disp->ogl_extras->ogl_info));
if (gl_disp->ogl_extras->ogl_info.version < _ALLEGRO_OPENGL_VERSION_3_0) {
ALLEGRO_DEBUG("OpenGL Extensions:\n");
print_extensions((char const *)glGetString(GL_EXTENSIONS));
}
#if defined(ALLEGRO_HAVE_DYNAMIC_LINK)
__libgl_handle = dlopen("libGL.so", RTLD_LAZY);
if (__libgl_handle) {
alXGetProcAddress = (GLXGETPROCADDRESSARBPROC) dlsym(__libgl_handle,
"glXGetProcAddressARB");
if (!alXGetProcAddress) {
alXGetProcAddress = (GLXGETPROCADDRESSARBPROC) dlsym(__libgl_handle,
"glXGetProcAddress");
if (!alXGetProcAddress) {
alXGetProcAddress = (GLXGETPROCADDRESSARBPROC) dlsym(__libgl_handle,
"eglGetProcAddress");
}
}
}
else {
ALLEGRO_WARN("Failed to dlopen libGL.so : %s\n", dlerror());
}
ALLEGRO_INFO("glXGetProcAddress Extension: %s\n",
alXGetProcAddress ? "Supported" : "Unsupported");
#elif defined ALLEGRO_UNIX
#if defined(ALLEGROGL_GLXGETPROCADDRESSARB)
ALLEGRO_INFO("glXGetProcAddressARB Extension: supported\n");
#else
ALLEGRO_INFO("glXGetProcAddress Extension: supported\n");
#endif
#endif
#if defined(ALLEGRO_MACOSX)
bundle_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
CFSTR
("/System/Library/Frameworks/OpenGL.framework"),
kCFURLPOSIXPathStyle, true);
opengl_bundle_ref = CFBundleCreate(kCFAllocatorDefault, bundle_url);
CFRelease(bundle_url);
#endif
#if defined ALLEGRO_UNIX && !defined ALLEGRO_EXCLUDE_GLX
ALLEGRO_DEBUG("GLX Extensions:\n");
ALLEGRO_SYSTEM_XGLX *glx_sys = (void*)al_get_system_driver();
ALLEGRO_DISPLAY_XGLX *glx_disp = (void *)gl_disp;
char const *ext = glXQueryExtensionsString(
glx_sys->gfxdisplay, glx_disp->xscreen);
if (!ext) {
ext = "";
}
print_extensions(ext);
#endif
ext_api = create_extension_api_table();
load_extensions(ext_api);
gl_disp->ogl_extras->extension_api = ext_api;
#if !defined ALLEGRO_CFG_OPENGLES
glGetStringi = ext_api->GetStringi;
if (gl_disp->ogl_extras->ogl_info.version >= _ALLEGRO_OPENGL_VERSION_3_0) {
ALLEGRO_DEBUG("OpenGL Extensions:\n");
print_extensions_3_0();
}
#endif
ext_list = create_extension_list();
gl_disp->ogl_extras->extension_list = ext_list;
#define AGL_EXT(name, ver) { ext_list->ALLEGRO_GL_##name = _ogl_is_extension_with_version_supported("GL_" #name, gl_disp, _ALLEGRO_OPENGL_VERSION_##ver); }
#include "allegro5/opengl/GLext/gl_ext_list.h"
#undef AGL_EXT
#if defined(ALLEGRO_UNIX)
#define AGL_EXT(name, ver) { ext_list->ALLEGRO_GLX_##name = _ogl_is_extension_with_version_supported("GLX_" #name, gl_disp, _ALLEGRO_OPENGL_VERSION_##ver); }
#include "allegro5/opengl/GLext/glx_ext_list.h"
#undef AGL_EXT
#elif defined ALLEGRO_WINDOWS
#define AGL_EXT(name, ver) { ext_list->ALLEGRO_WGL_##name = _ogl_is_extension_with_version_supported("WGL_" #name, gl_disp, _ALLEGRO_OPENGL_VERSION_##ver); }
#include "allegro5/opengl/GLext/wgl_ext_list.h"
#undef AGL_EXT
#endif
glGetIntegerv(GL_MAX_TEXTURE_SIZE,
(GLint *) & gl_disp->ogl_extras->ogl_info.max_texture_size);
ext_list->ALLEGRO_GL_EXT_packed_pixels &= !gl_disp->ogl_extras->ogl_info.is_voodoo;
if (ext_list->ALLEGRO_GL_EXT_packed_pixels) {
ALLEGRO_INFO("Packed Pixels formats available\n");
#if 0
__allegro_gl_texture_read_format[0] = GL_UNSIGNED_BYTE_3_3_2;
__allegro_gl_texture_read_format[1] = GL_UNSIGNED_SHORT_5_5_5_1;
__allegro_gl_texture_read_format[2] = GL_UNSIGNED_SHORT_5_6_5;
#endif 
}
{
const char *vendor = (const char *)glGetString(GL_VENDOR);
if (strstr(vendor, "NVIDIA Corporation")) {
if (!ext_list->ALLEGRO_GL_NV_fragment_program2
|| !ext_list->ALLEGRO_GL_NV_vertex_program3) {
ext_list->ALLEGRO_GL_ARB_texture_non_power_of_two = 0;
}
}
else if (strstr(vendor, "ATI Technologies")) {
if (gl_disp->ogl_extras->ogl_info.version >= _ALLEGRO_OPENGL_VERSION_3_0) {
}
else if (!strstr((const char *)glGetString(GL_EXTENSIONS),
"GL_ARB_texture_non_power_of_two")
&& gl_disp->ogl_extras->ogl_info.version >= _ALLEGRO_OPENGL_VERSION_2_0) {
ext_list->ALLEGRO_GL_ARB_texture_non_power_of_two = 0;
}
}
}
{
int *s = gl_disp->extra_settings.settings;
glGetIntegerv(GL_MAX_TEXTURE_SIZE, s + ALLEGRO_MAX_BITMAP_SIZE);
if (gl_disp->ogl_extras->ogl_info.version >= _ALLEGRO_OPENGL_VERSION_2_0)
s[ALLEGRO_SUPPORT_SEPARATE_ALPHA] = 1;
s[ALLEGRO_SUPPORT_NPOT_BITMAP] =
ext_list->ALLEGRO_GL_ARB_texture_non_power_of_two ||
ext_list->ALLEGRO_GL_OES_texture_npot;
ALLEGRO_INFO("Use of non-power-of-two textures %s.\n",
s[ALLEGRO_SUPPORT_NPOT_BITMAP] ? "enabled" : "disabled");
#if defined ALLEGRO_CFG_OPENGLES
if (gl_disp->flags & ALLEGRO_PROGRAMMABLE_PIPELINE) {
s[ALLEGRO_CAN_DRAW_INTO_BITMAP] = true;
}
else {
s[ALLEGRO_CAN_DRAW_INTO_BITMAP] =
ext_list->ALLEGRO_GL_OES_framebuffer_object;
}
ALLEGRO_INFO("Use of FBO to draw to textures %s.\n",
s[ALLEGRO_CAN_DRAW_INTO_BITMAP] ? "enabled" :
"disabled");
#else
s[ALLEGRO_CAN_DRAW_INTO_BITMAP] =
ext_list->ALLEGRO_GL_EXT_framebuffer_object;
ALLEGRO_INFO("Use of FBO to draw to textures %s.\n",
s[ALLEGRO_CAN_DRAW_INTO_BITMAP] ? "enabled" :
"disabled");
#endif
}
}
ALLEGRO_OGL_EXT_LIST *al_get_opengl_extension_list(void)
{
ALLEGRO_DISPLAY *disp;
disp = al_get_current_display();
ASSERT(disp);
if (!(disp->flags & ALLEGRO_OPENGL))
return NULL;
ASSERT(disp->ogl_extras);
return disp->ogl_extras->extension_list;
}
void _al_ogl_unmanage_extensions(ALLEGRO_DISPLAY *gl_disp)
{
destroy_extension_api_table(gl_disp->ogl_extras->extension_api);
destroy_extension_list(gl_disp->ogl_extras->extension_list);
gl_disp->ogl_extras->extension_api = NULL;
gl_disp->ogl_extras->extension_list = NULL;
#if defined(ALLEGRO_MACOSX)
CFRelease(opengl_bundle_ref);
#endif
#if defined(ALLEGRO_HAVE_DYNAMIC_LINK)
if (__libgl_handle) {
dlclose(__libgl_handle);
__libgl_handle = NULL;
}
#endif
}
