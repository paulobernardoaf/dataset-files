





















#include <jni.h>

#include "ffjni.h"
#include "mediacodec_surface.h"

FFANativeWindow *ff_mediacodec_surface_ref(void *surface, void *log_ctx)
{
JNIEnv *env = NULL;

env = ff_jni_get_env(log_ctx);
if (!env) {
return NULL;
}

return (*env)->NewGlobalRef(env, surface);
}

int ff_mediacodec_surface_unref(FFANativeWindow *window, void *log_ctx)
{
JNIEnv *env = NULL;

env = ff_jni_get_env(log_ctx);
if (!env) {
return AVERROR_EXTERNAL;
}

(*env)->DeleteGlobalRef(env, window);

return 0;
}
