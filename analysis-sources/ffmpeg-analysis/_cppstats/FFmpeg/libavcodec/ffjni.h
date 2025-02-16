#include <jni.h>
JNIEnv *ff_jni_get_env(void *log_ctx);
char *ff_jni_jstring_to_utf_chars(JNIEnv *env, jstring string, void *log_ctx);
jstring ff_jni_utf_chars_to_jstring(JNIEnv *env, const char *utf_chars, void *log_ctx);
int ff_jni_exception_get_summary(JNIEnv *env, jthrowable exception, char **error, void *log_ctx);
int ff_jni_exception_check(JNIEnv *env, int log, void *log_ctx);
enum FFJniFieldType {
FF_JNI_CLASS,
FF_JNI_FIELD,
FF_JNI_STATIC_FIELD,
FF_JNI_METHOD,
FF_JNI_STATIC_METHOD
};
struct FFJniField {
const char *name;
const char *method;
const char *signature;
enum FFJniFieldType type;
int offset;
int mandatory;
};
int ff_jni_init_jfields(JNIEnv *env, void *jfields, const struct FFJniField *jfields_mapping, int global, void *log_ctx);
int ff_jni_reset_jfields(JNIEnv *env, void *jfields, const struct FFJniField *jfields_mapping, int global, void *log_ctx);
