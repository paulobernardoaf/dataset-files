






















#if !defined(VLC_GL_H)
#define VLC_GL_H 1






struct vout_window_t;
struct vout_window_cfg_t;
struct vout_display_cfg;




typedef struct vlc_gl_t vlc_gl_t;

struct vlc_gl_t
{
struct vlc_object_t obj;

struct vout_window_t *surface;
module_t *module;
void *sys;

int (*makeCurrent)(vlc_gl_t *);
void (*releaseCurrent)(vlc_gl_t *);
void (*resize)(vlc_gl_t *, unsigned, unsigned);
void (*swap)(vlc_gl_t *);
void*(*getProcAddress)(vlc_gl_t *, const char *);
void (*destroy)(vlc_gl_t *);

enum {
VLC_GL_EXT_DEFAULT,
VLC_GL_EXT_EGL,
VLC_GL_EXT_WGL,
} ext;

union {

struct {

const char *(*queryString)(vlc_gl_t *, int32_t name);


void *(*createImageKHR)(vlc_gl_t *, unsigned target, void *buffer,
const int32_t *attrib_list);

bool (*destroyImageKHR)(vlc_gl_t *, void *image);
} egl;

struct
{
const char *(*getExtensionsString)(vlc_gl_t *);
} wgl;
};
};

enum {
VLC_OPENGL,
VLC_OPENGL_ES2,
};











VLC_API vlc_gl_t *vlc_gl_Create(const struct vout_display_cfg *cfg,
unsigned flags, const char *name) VLC_USED;
VLC_API void vlc_gl_Release(vlc_gl_t *);
VLC_API void vlc_gl_Hold(vlc_gl_t *);

static inline int vlc_gl_MakeCurrent(vlc_gl_t *gl)
{
return gl->makeCurrent(gl);
}

static inline void vlc_gl_ReleaseCurrent(vlc_gl_t *gl)
{
gl->releaseCurrent(gl);
}

static inline void vlc_gl_Resize(vlc_gl_t *gl, unsigned w, unsigned h)
{
if (gl->resize != NULL)
gl->resize(gl, w, h);
}

static inline void vlc_gl_Swap(vlc_gl_t *gl)
{
gl->swap(gl);
}

static inline void *vlc_gl_GetProcAddress(vlc_gl_t *gl, const char *name)
{
return gl->getProcAddress(gl, name);
}

VLC_API vlc_gl_t *vlc_gl_surface_Create(vlc_object_t *,
const struct vout_window_cfg_t *,
struct vout_window_t **) VLC_USED;
VLC_API bool vlc_gl_surface_CheckSize(vlc_gl_t *, unsigned *w, unsigned *h);
VLC_API void vlc_gl_surface_Destroy(vlc_gl_t *);

static inline bool vlc_gl_StrHasToken(const char *apis, const char *api)
{
size_t apilen = strlen(api);
while (apis) {
while (*apis == ' ')
apis++;
if (!strncmp(apis, api, apilen) && memchr(" ", apis[apilen], 2))
return true;
apis = strchr(apis, ' ');
}
return false;
}

#endif 
