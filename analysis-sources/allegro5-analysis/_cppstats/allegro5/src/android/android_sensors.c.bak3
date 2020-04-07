














#include "allegro5/allegro.h"
#include "allegro5/allegro_android.h"
#include "allegro5/internal/aintern_android.h"

#include <jni.h>

JNI_FUNC(void, Sensors, nativeOnAccel, (JNIEnv *env, jobject obj, jint id,
jfloat x, jfloat y, jfloat z))
{
(void)env;
(void)obj;
(void)id;
_al_android_generate_accelerometer_event(x, y, z);
}


