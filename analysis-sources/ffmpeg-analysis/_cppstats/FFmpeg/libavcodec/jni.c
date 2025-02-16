#include "config.h"
#include <stdlib.h>
#include "libavutil/error.h"
#include "jni.h"
#if CONFIG_JNI
#include <jni.h>
#include <pthread.h>
#include "libavutil/log.h"
#include "ffjni.h"
void *java_vm;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int av_jni_set_java_vm(void *vm, void *log_ctx)
{
int ret = 0;
pthread_mutex_lock(&lock);
if (java_vm == NULL) {
java_vm = vm;
} else if (java_vm != vm) {
ret = AVERROR(EINVAL);
av_log(log_ctx, AV_LOG_ERROR, "A Java virtual machine has already been set");
}
pthread_mutex_unlock(&lock);
return ret;
}
void *av_jni_get_java_vm(void *log_ctx)
{
void *vm;
pthread_mutex_lock(&lock);
vm = java_vm;
pthread_mutex_unlock(&lock);
return vm;
}
#else
int av_jni_set_java_vm(void *vm, void *log_ctx)
{
return AVERROR(ENOSYS);
}
void *av_jni_get_java_vm(void *log_ctx)
{
return NULL;
}
#endif
