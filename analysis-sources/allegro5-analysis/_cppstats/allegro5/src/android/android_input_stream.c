#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_android.h"
#include <jni.h>
ALLEGRO_DEBUG_CHANNEL("android")
ALLEGRO_STATIC_ASSERT(android, sizeof(jlong) >= sizeof(ALLEGRO_FILE *));
JNI_FUNC(int, AllegroInputStream, nativeRead, (JNIEnv *env, jobject obj,
jlong handle, jbyteArray array, int offset, int length))
{
ALLEGRO_FILE *fp = (ALLEGRO_FILE *)(intptr_t)handle;
int ret = -1;
jbyte *array_ptr = NULL;
ASSERT(fp != NULL);
(void)obj;
ALLEGRO_DEBUG("nativeRead begin: handle:%lli fp:%p offset:%i length:%i",
handle, fp, offset, length);
int array_len = _jni_call(env, int, GetArrayLength, array);
ALLEGRO_DEBUG("array length: %i", array_len);
array_ptr = _jni_call(env, jbyte *, GetByteArrayElements, array, NULL);
ASSERT(array_ptr != NULL);
ALLEGRO_DEBUG("al_fread: p:%p, o:%i, l:%i", array_ptr, offset, length);
ret = al_fread(fp, array_ptr + offset, length);
if (ret == 0 && al_feof(fp)) {
ret = -1;
}
_jni_callv(env, ReleaseByteArrayElements, array, array_ptr, 0);
ALLEGRO_DEBUG("nativeRead end");
return ret;
}
JNI_FUNC(void, AllegroInputStream, nativeClose, (JNIEnv *env, jobject obj,
jlong handle))
{
ALLEGRO_FILE *fp = (ALLEGRO_FILE *)(intptr_t)handle;
(void)env;
(void)obj;
al_fclose(fp);
}
