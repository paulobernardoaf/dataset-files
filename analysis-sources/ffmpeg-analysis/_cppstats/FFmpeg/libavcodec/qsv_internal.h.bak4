



















#if !defined(AVCODEC_QSV_INTERNAL_H)
#define AVCODEC_QSV_INTERNAL_H

#if CONFIG_VAAPI
#define AVCODEC_QSV_LINUX_SESSION_HANDLE
#endif 

#if defined(AVCODEC_QSV_LINUX_SESSION_HANDLE)
#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <va/va.h>
#include <va/va_drm.h>
#include "libavutil/hwcontext_vaapi.h"
#endif

#include <mfx/mfxvideo.h>

#include "libavutil/frame.h"

#include "avcodec.h"

#define QSV_VERSION_MAJOR 1
#define QSV_VERSION_MINOR 1

#define ASYNC_DEPTH_DEFAULT 4 

#define QSV_MAX_ENC_PAYLOAD 2 

#define QSV_VERSION_ATLEAST(MAJOR, MINOR) (MFX_VERSION_MAJOR > (MAJOR) || MFX_VERSION_MAJOR == (MAJOR) && MFX_VERSION_MINOR >= (MINOR))



#define QSV_RUNTIME_VERSION_ATLEAST(MFX_VERSION, MAJOR, MINOR) ((MFX_VERSION.Major > (MAJOR)) || (MFX_VERSION.Major == (MAJOR) && MFX_VERSION.Minor >= (MINOR)))



typedef struct QSVMid {
AVBufferRef *hw_frames_ref;
mfxHDL handle;

AVFrame *locked_frame;
AVFrame *hw_frame;
mfxFrameSurface1 surf;
} QSVMid;

typedef struct QSVFrame {
AVFrame *frame;
mfxFrameSurface1 surface;
mfxEncodeCtrl enc_ctrl;
mfxExtDecodedFrameInfo dec_info;
mfxExtBuffer *ext_param;

int queued;
int used;

struct QSVFrame *next;
} QSVFrame;

typedef struct QSVSession {
mfxSession session;
#if defined(AVCODEC_QSV_LINUX_SESSION_HANDLE)
AVBufferRef *va_device_ref;
AVHWDeviceContext *va_device_ctx;
#endif
} QSVSession;

typedef struct QSVFramesContext {
AVBufferRef *hw_frames_ctx;
void *logctx;





AVBufferRef *mids_buf;
QSVMid *mids;
int nb_mids;
} QSVFramesContext;

int ff_qsv_print_iopattern(void *log_ctx, int mfx_iopattern,
const char *extra_string);




int ff_qsv_map_error(mfxStatus mfx_err, const char **desc);

int ff_qsv_print_error(void *log_ctx, mfxStatus err,
const char *error_string);

int ff_qsv_print_warning(void *log_ctx, mfxStatus err,
const char *warning_string);

int ff_qsv_codec_id_to_mfx(enum AVCodecID codec_id);
int ff_qsv_level_to_mfx(enum AVCodecID codec_id, int level);

enum AVPixelFormat ff_qsv_map_fourcc(uint32_t fourcc);

int ff_qsv_map_pixfmt(enum AVPixelFormat format, uint32_t *fourcc);
enum AVPictureType ff_qsv_map_pictype(int mfx_pic_type);

enum AVFieldOrder ff_qsv_map_picstruct(int mfx_pic_struct);

int ff_qsv_init_internal_session(AVCodecContext *avctx, QSVSession *qs,
const char *load_plugins, int gpu_copy);

int ff_qsv_close_internal_session(QSVSession *qs);

int ff_qsv_init_session_device(AVCodecContext *avctx, mfxSession *psession,
AVBufferRef *device_ref, const char *load_plugins,
int gpu_copy);

int ff_qsv_init_session_frames(AVCodecContext *avctx, mfxSession *session,
QSVFramesContext *qsv_frames_ctx,
const char *load_plugins, int opaque, int gpu_copy);

int ff_qsv_find_surface_idx(QSVFramesContext *ctx, QSVFrame *frame);

#endif 
