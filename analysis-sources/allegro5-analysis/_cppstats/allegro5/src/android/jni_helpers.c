#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_android.h"
#include <jni.h>
#include <stdarg.h>
ALLEGRO_DEBUG_CHANNEL("jni")
#define VERBOSE_DEBUG(a, ...) (void)0
void __jni_checkException(JNIEnv *env, const char *file, const char *func, int line)
{
jthrowable exc;
exc = (*env)->ExceptionOccurred(env);
if (exc) {
ALLEGRO_DEBUG("GOT AN EXCEPTION @ %s:%i %s", file, line, func);
(*env)->ExceptionDescribe(env);
(*env)->ExceptionClear(env);
(*env)->FatalError(env, "EXCEPTION");
}
}
jobject _jni_callObjectMethod(JNIEnv *env, jobject object,
const char *name, const char *sig)
{
VERBOSE_DEBUG("%s (%s)", name, sig);
jclass class_id = _jni_call(env, jclass, GetObjectClass, object);
jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig);
jobject ret = _jni_call(env, jobject, CallObjectMethod, object, method_id);
_jni_callv(env, DeleteLocalRef, class_id);
return ret;
}
jobject _jni_callObjectMethodV(JNIEnv *env, jobject object,
const char *name, const char *sig, ...)
{
va_list ap;
VERBOSE_DEBUG("%s (%s)", name, sig);
jclass class_id = _jni_call(env, jclass, GetObjectClass, object);
jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig);
va_start(ap, sig);
jobject ret = _jni_call(env, jobject, CallObjectMethodV, object, method_id, ap);
va_end(ap);
_jni_callv(env, DeleteLocalRef, class_id);
VERBOSE_DEBUG("callObjectMethodV end");
return ret;
}
ALLEGRO_USTR *_jni_getString(JNIEnv *env, jobject object)
{
VERBOSE_DEBUG("GetStringUTFLength");
jsize len = _jni_call(env, jsize, GetStringUTFLength, object);
const char *str = _jni_call(env, const char *, GetStringUTFChars, object, NULL);
VERBOSE_DEBUG("al_ustr_new_from_buffer");
ALLEGRO_USTR *ustr = al_ustr_new_from_buffer(str, len);
_jni_callv(env, ReleaseStringUTFChars, object, str);
return ustr;
}
ALLEGRO_USTR *_jni_callStringMethod(JNIEnv *env, jobject obj,
const char *name, const char *sig)
{
jobject str_obj = _jni_callObjectMethod(env, obj, name, sig);
return _jni_getString(env, str_obj);
}
jobject _jni_callStaticObjectMethodV(JNIEnv *env, jclass cls,
const char *name, const char *sig, ...)
{
jmethodID mid;
jobject ret;
va_list ap;
mid = _jni_call(env, jmethodID, GetStaticMethodID, cls, name, sig);
va_start(ap, sig);
ret = _jni_call(env, jobject, CallStaticObjectMethodV, cls, mid, ap);
va_end(ap);
return ret;
}
jint _jni_callStaticIntMethodV(JNIEnv *env, jclass cls,
const char *name, const char *sig, ...)
{
jmethodID mid;
jint ret;
va_list ap;
mid = _jni_call(env, jmethodID, GetStaticMethodID, cls, name, sig);
va_start(ap, sig);
ret = _jni_call(env, jint, CallStaticIntMethodV, cls, mid, ap);
va_end(ap);
return ret;
}
