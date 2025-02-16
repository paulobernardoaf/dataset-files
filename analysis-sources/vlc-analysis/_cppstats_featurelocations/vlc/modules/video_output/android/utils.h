






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/input.h>

#include <vlc_vout_display.h>
#include <vlc_common.h>

typedef struct AWindowHandler AWindowHandler;

enum AWindow_ID {
AWindow_Video,
AWindow_Subtitles,
AWindow_SurfaceTexture,
AWindow_Max,
};




typedef struct
{
int32_t (*winLock)(ANativeWindow*, ANativeWindow_Buffer*, ARect*);
void (*unlockAndPost)(ANativeWindow*);
int32_t (*setBuffersGeometry)(ANativeWindow*, int32_t, int32_t, int32_t); 
} native_window_api_t;

struct awh_mouse_coords
{
int i_action;
int i_button;
int i_x;
int i_y;
};

typedef struct
{
void (*on_new_window_size)(vout_window_t *wnd, unsigned i_width,
unsigned i_height);
void (*on_new_mouse_coords)(vout_window_t *wnd,
const struct awh_mouse_coords *coords);
} awh_events_t;

typedef struct android_video_context_t android_video_context_t;

struct android_video_context_t
{
enum AWindow_ID id;
void *dec_opaque;
bool (*render)(struct picture_context_t *ctx);
bool (*render_ts)(struct picture_context_t *ctx, vlc_tick_t ts);
};








JNIEnv *android_getEnv(vlc_object_t *p_obj, const char *psz_thread_name);









AWindowHandler *AWindowHandler_new(vout_window_t *wnd, awh_events_t *p_events);
void AWindowHandler_destroy(AWindowHandler *p_awh);







native_window_api_t *AWindowHandler_getANativeWindowAPI(AWindowHandler *p_awh);







jobject AWindowHandler_getSurface(AWindowHandler *p_awh, enum AWindow_ID id);







ANativeWindow *AWindowHandler_getANativeWindow(AWindowHandler *p_awh,
enum AWindow_ID id);




void AWindowHandler_releaseANativeWindow(AWindowHandler *p_awh,
enum AWindow_ID id);




bool AWindowHandler_canSetVideoLayout(AWindowHandler *p_awh);






int AWindowHandler_setVideoLayout(AWindowHandler *p_awh,
int i_width, int i_height,
int i_visible_width, int i_visible_height,
int i_sar_num, int i_sar_den);








int
SurfaceTexture_attachToGLContext(AWindowHandler *p_awh, int tex_name);





void
SurfaceTexture_detachFromGLContext(AWindowHandler *p_awh);






static inline jobject
SurfaceTexture_getSurface(AWindowHandler *p_awh)
{
return AWindowHandler_getSurface(p_awh, AWindow_SurfaceTexture);
}






static inline ANativeWindow *
SurfaceTexture_getANativeWindow(AWindowHandler *p_awh)
{
return AWindowHandler_getANativeWindow(p_awh, AWindow_SurfaceTexture);
}














int
SurfaceTexture_waitAndUpdateTexImage(AWindowHandler *p_awh,
const float **pp_transform_mtx);
