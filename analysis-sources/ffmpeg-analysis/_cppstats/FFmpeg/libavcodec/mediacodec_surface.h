#include "libavcodec/avcodec.h"
struct FFANativeWindow;
typedef struct FFANativeWindow FFANativeWindow;
FFANativeWindow *ff_mediacodec_surface_ref(void *surface, void *log_ctx);
int ff_mediacodec_surface_unref(FFANativeWindow *window, void *log_ctx);
