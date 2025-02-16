#include <assert.h>
#if !defined(__cplusplus)
#include <stdatomic.h>
#else
#include <atomic>
using std::atomic_uintptr_t;
using std::memory_order_relaxed;
using std::memory_order_release;
#endif
#include <vlc_es.h>
typedef struct plane_t
{
uint8_t *p_pixels; 
int i_lines; 
int i_pitch; 
int i_pixel_pitch;
int i_visible_lines; 
int i_visible_pitch; 
} plane_t;
#define PICTURE_PLANE_MAX (VOUT_MAX_PLANES)
typedef struct picture_context_t
{
void (*destroy)(struct picture_context_t *);
struct picture_context_t *(*copy)(struct picture_context_t *);
struct vlc_video_context *vctx;
} picture_context_t;
typedef struct picture_buffer_t
{
int fd;
void *base;
size_t size;
off_t offset;
} picture_buffer_t;
typedef struct vlc_decoder_device vlc_decoder_device;
typedef struct vlc_video_context vlc_video_context;
struct vlc_video_context_operations
{
void (*destroy)(void *priv);
};
enum vlc_video_context_type
{
VLC_VIDEO_CONTEXT_NONE,
VLC_VIDEO_CONTEXT_VAAPI,
VLC_VIDEO_CONTEXT_VDPAU,
VLC_VIDEO_CONTEXT_DXVA2, 
VLC_VIDEO_CONTEXT_D3D11VA, 
VLC_VIDEO_CONTEXT_AWINDOW, 
VLC_VIDEO_CONTEXT_NVDEC,
VLC_VIDEO_CONTEXT_CVPX,
VLC_VIDEO_CONTEXT_MMAL,
};
VLC_API vlc_video_context * vlc_video_context_Create(vlc_decoder_device *,
enum vlc_video_context_type private_type,
size_t private_size,
const struct vlc_video_context_operations *);
VLC_API void vlc_video_context_Release(vlc_video_context *);
VLC_API enum vlc_video_context_type vlc_video_context_GetType(const vlc_video_context *);
VLC_API void *vlc_video_context_GetPrivate(vlc_video_context *, enum vlc_video_context_type);
VLC_API vlc_video_context *vlc_video_context_Hold(vlc_video_context *);
VLC_API vlc_decoder_device *vlc_video_context_HoldDevice(vlc_video_context *);
struct picture_t
{
video_frame_format_t format;
plane_t p[PICTURE_PLANE_MAX]; 
int i_planes; 
vlc_tick_t date; 
bool b_force;
bool b_still;
bool b_progressive; 
bool b_top_field_first; 
unsigned int i_nb_fields; 
picture_context_t *context; 
void *p_sys;
struct picture_t *p_next;
atomic_uintptr_t refs;
};
static inline vlc_video_context* picture_GetVideoContext(picture_t *pic)
{
return pic->context ? pic->context->vctx : NULL;
}
VLC_API picture_t * picture_New( vlc_fourcc_t i_chroma, int i_width, int i_height, int i_sar_num, int i_sar_den ) VLC_USED;
VLC_API picture_t * picture_NewFromFormat( const video_format_t *p_fmt ) VLC_USED;
typedef struct
{
void *p_sys;
void (*pf_destroy)(picture_t *);
struct
{
uint8_t *p_pixels; 
int i_lines; 
int i_pitch; 
} p[PICTURE_PLANE_MAX];
} picture_resource_t;
VLC_API picture_t * picture_NewFromResource( const video_format_t *, const picture_resource_t * ) VLC_USED;
VLC_API void picture_Destroy(picture_t *picture);
static inline picture_t *picture_Hold(picture_t *picture)
{
atomic_fetch_add_explicit(&picture->refs, (uintptr_t)1,
memory_order_relaxed);
return picture;
}
static inline void picture_Release(picture_t *picture)
{
uintptr_t refs = atomic_fetch_sub_explicit(&picture->refs, (uintptr_t)1,
memory_order_release);
vlc_assert(refs > 0);
if (refs == 1)
picture_Destroy(picture);
}
VLC_API void picture_CopyProperties( picture_t *p_dst, const picture_t *p_src );
VLC_API void picture_Reset( picture_t * );
VLC_API void picture_CopyPixels( picture_t *p_dst, const picture_t *p_src );
VLC_API void plane_CopyPixels( plane_t *p_dst, const plane_t *p_src );
VLC_API void picture_Copy( picture_t *p_dst, const picture_t *p_src );
VLC_API picture_t *picture_Clone(picture_t *pic);
VLC_API int picture_Export( vlc_object_t *p_obj, block_t **pp_image, video_format_t *p_fmt,
picture_t *p_picture, vlc_fourcc_t i_format, int i_override_width,
int i_override_height, bool b_crop );
VLC_API int picture_Setup( picture_t *, const video_format_t * );
enum
{
Y_PLANE = 0,
U_PLANE = 1,
V_PLANE = 2,
A_PLANE = 3,
};
#define Y_PIXELS p[Y_PLANE].p_pixels
#define Y_PITCH p[Y_PLANE].i_pitch
#define U_PIXELS p[U_PLANE].p_pixels
#define U_PITCH p[U_PLANE].i_pitch
#define V_PIXELS p[V_PLANE].p_pixels
#define V_PITCH p[V_PLANE].i_pitch
#define A_PIXELS p[A_PLANE].p_pixels
#define A_PITCH p[A_PLANE].i_pitch
static inline void picture_SwapUV(picture_t *picture)
{
vlc_assert(picture->i_planes == 3);
plane_t tmp_plane = picture->p[U_PLANE];
picture->p[U_PLANE] = picture->p[V_PLANE];
picture->p[V_PLANE] = tmp_plane;
}
