












#if !defined(A5_ANDROID_ALLEGRO_H)
#define A5_ANDROID_ALLEGRO_H

#include <jni.h>

#if defined(__cplusplus)
extern "C" {
#endif




void al_android_set_apk_file_interface(void);
const char *al_android_get_os_version(void);
void al_android_set_apk_fs_interface(void);
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
JNIEnv *al_android_get_jni_env(void);
jobject al_android_get_activity(void);
#endif


void _al_android_set_capture_volume_keys(ALLEGRO_DISPLAY *display, bool onoff);

#if defined(__cplusplus)
}
#endif

#endif 
