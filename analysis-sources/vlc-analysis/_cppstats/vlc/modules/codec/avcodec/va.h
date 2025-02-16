#include "avcommon_compat.h"
#include <libavutil/pixdesc.h>
typedef struct vlc_va_t vlc_va_t;
typedef struct vlc_va_sys_t vlc_va_sys_t;
typedef struct vlc_decoder_device vlc_decoder_device;
typedef struct vlc_video_context vlc_video_context;
struct vlc_va_operations {
int (*get)(vlc_va_t *, picture_t *pic, uint8_t **surface);
void (*close)(vlc_va_t *);
};
struct vlc_va_t {
struct vlc_object_t obj;
vlc_va_sys_t *sys;
const struct vlc_va_operations *ops;
};
typedef int (*vlc_va_open)(vlc_va_t *, AVCodecContext *,
enum PixelFormat hwfmt, const AVPixFmtDescriptor *,
const es_format_t *, vlc_decoder_device *,
video_format_t *, vlc_video_context **);
#define set_va_callback(activate, priority) { vlc_va_open open__ = activate; (void) open__; set_callback(activate) } set_capability( "hw decoder", priority )
bool vlc_va_MightDecode(enum PixelFormat hwfmt, enum PixelFormat swfmt);
vlc_va_t *vlc_va_New(vlc_object_t *obj, AVCodecContext *,
enum PixelFormat hwfmt, const AVPixFmtDescriptor *,
const es_format_t *fmt, vlc_decoder_device *device,
video_format_t *, vlc_video_context **vtcx_out);
static inline int vlc_va_Get(vlc_va_t *va, picture_t *pic, uint8_t **surface)
{
return va->ops->get(va, pic, surface);
}
void vlc_va_Delete(vlc_va_t *);
