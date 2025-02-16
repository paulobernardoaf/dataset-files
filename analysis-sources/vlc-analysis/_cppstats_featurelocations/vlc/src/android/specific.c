



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>

#include <vlc_common.h>
#include <vlc_network.h>
#include <vlc_fs.h>
#include "../libvlc.h"
#include "config/configuration.h"

#include <string.h>
#include <jni.h>

static JavaVM *s_jvm = NULL;
#define GENERIC_DIR_COUNT (VLC_VIDEOS_DIR - VLC_DESKTOP_DIR + 1)
static char *ppsz_generic_names[GENERIC_DIR_COUNT] = {};
static struct {
struct {
jclass clazz;
jmethodID getExternalStoragePublicDirectory;
} Environment;
struct {
jmethodID getAbsolutePath;
} File;
struct {
jclass clazz;
jmethodID getProperty;
} System;
} fields = { .Environment.clazz = NULL };

static char *
get_java_string(JNIEnv *env, jclass clazz, const char *psz_name)
{
jfieldID id = (*env)->GetStaticFieldID(env, clazz, psz_name,
"Ljava/lang/String;");
if ((*env)->ExceptionCheck(env))
{
(*env)->ExceptionClear(env);
return NULL;
}

jstring jstr = (*env)->GetStaticObjectField(env, clazz, id);

const char *psz_str = (*env)->GetStringUTFChars(env, jstr, 0);
if (psz_str == NULL)
return NULL;

char *psz_strdup = strdup(psz_str);

(*env)->ReleaseStringUTFChars(env, jstr, psz_str);
(*env)->DeleteLocalRef(env, jstr);

return psz_strdup;
}

void
JNI_OnUnload(JavaVM* vm, void* reserved)
{
(void) reserved;

for (size_t i = 0; i < GENERIC_DIR_COUNT; ++i)
free(ppsz_generic_names[i]);

JNIEnv* env = NULL;
if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_2) != JNI_OK)
return;

if (fields.Environment.clazz)
(*env)->DeleteGlobalRef(env, fields.Environment.clazz);

if (fields.System.clazz)
(*env)->DeleteGlobalRef(env, fields.System.clazz);
}




jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
s_jvm = vm;
JNIEnv* env = NULL;

if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_2) != JNI_OK)
return -1;

jclass clazz = (*env)->FindClass(env, "android/os/Environment");
if ((*env)->ExceptionCheck(env))
return -1;

static const char *ppsz_env_names[GENERIC_DIR_COUNT] = {
NULL, 
"DIRECTORY_DOWNLOADS", 
NULL, 
NULL, 
"DIRECTORY_DOCUMENTS", 
"DIRECTORY_MUSIC", 
"DIRECTORY_PICTURES", 
"DIRECTORY_MOVIES", 
};
for (size_t i = 0; i < GENERIC_DIR_COUNT; ++i)
{
if (ppsz_env_names[i] != NULL)
ppsz_generic_names[i] = get_java_string(env, clazz,
ppsz_env_names[i]);
}

fields.Environment.clazz = (*env)->NewGlobalRef(env, clazz);
fields.Environment.getExternalStoragePublicDirectory =
(*env)->GetStaticMethodID(env, clazz,
"getExternalStoragePublicDirectory",
"(Ljava/lang/String;)Ljava/io/File;");
if ((*env)->ExceptionCheck(env))
goto error;
(*env)->DeleteLocalRef(env, clazz);

clazz = (*env)->FindClass(env, "java/io/File");
fields.File.getAbsolutePath =
(*env)->GetMethodID(env, clazz, "getAbsolutePath",
"()Ljava/lang/String;");
if ((*env)->ExceptionCheck(env))
goto error;
(*env)->DeleteLocalRef(env, clazz);

clazz = (*env)->FindClass(env, "java/lang/System");
if ((*env)->ExceptionCheck(env))
goto error;
fields.System.clazz = (*env)->NewGlobalRef(env, clazz);
fields.System.getProperty =
(*env)->GetStaticMethodID(env, clazz, "getProperty",
"(Ljava/lang/String;)Ljava/lang/String;");
(*env)->DeleteLocalRef(env, clazz);

return JNI_VERSION_1_2;

error:
if (clazz)
(*env)->DeleteLocalRef(env, clazz);
JNI_OnUnload(vm, reserved);
return -1;
}

void
system_Init(void)
{
}

void
system_Configure(libvlc_int_t *p_libvlc, int i_argc, const char *const pp_argv[])
{
(void)i_argc; (void)pp_argv;
assert(s_jvm != NULL);
var_Create(p_libvlc, "android-jvm", VLC_VAR_ADDRESS);
var_SetAddress(p_libvlc, "android-jvm", s_jvm);
}

static char *config_GetHomeDir(const char *psz_dir, const char *psz_default_dir)
{
char *psz_home = getenv("HOME");
if (psz_home == NULL)
goto fallback;

if (psz_dir == NULL)
return strdup(psz_home);

char *psz_fullpath;
if (asprintf(&psz_fullpath, "%s/%s", psz_home, psz_dir) == -1)
goto fallback;
if (vlc_mkdir(psz_fullpath, 0700) == -1 && errno != EEXIST)
{
free(psz_fullpath);
goto fallback;
}
return psz_fullpath;

fallback:
return psz_default_dir != NULL ? strdup(psz_default_dir) : NULL;
}

static JNIEnv *get_env(bool *p_detach)
{
JNIEnv *env;
if ((*s_jvm)->GetEnv(s_jvm, (void **)&env, JNI_VERSION_1_2) != JNI_OK)
{

JavaVMAttachArgs args;

args.version = JNI_VERSION_1_2;
args.name = "config_GetGenericDir";
args.group = NULL;

if ((*s_jvm)->AttachCurrentThread(s_jvm, &env, &args) != JNI_OK)
return NULL;
*p_detach = true;
}
else
*p_detach = false;
return env;
}

static void release_env(bool b_detach)
{
if (b_detach)
(*s_jvm)->DetachCurrentThread(s_jvm);
}

static char *config_GetGenericDir(const char *psz_name)
{
JNIEnv *env;
bool b_detach;
char *psz_ret = NULL;

env = get_env(&b_detach);
if (env == NULL)
return NULL;

jstring jname= (*env)->NewStringUTF(env, psz_name);
if ((*env)->ExceptionCheck(env))
{
(*env)->ExceptionClear(env);
jname = NULL;
}
if (jname == NULL)
goto error;

jobject jfile = (*env)->CallStaticObjectMethod(env,
fields.Environment.clazz,
fields.Environment.getExternalStoragePublicDirectory,
jname);
(*env)->DeleteLocalRef(env, jname);
if (jfile == NULL)
goto error;

jstring jpath = (*env)->CallObjectMethod(env, jfile,
fields.File.getAbsolutePath);
(*env)->DeleteLocalRef(env, jfile);

const char *psz_path = (*env)->GetStringUTFChars(env, jpath, 0);
if (psz_path == NULL)
goto error;
psz_ret = strdup(psz_path);
(*env)->ReleaseStringUTFChars(env, jpath, psz_path);
(*env)->DeleteLocalRef(env, jpath);

error:
release_env(b_detach);
return psz_ret;
}

char *config_GetUserDir (vlc_userdir_t type)
{
switch (type)
{
case VLC_USERDATA_DIR:
return config_GetHomeDir(".share",
"/sdcard/Android/data/org.videolan.vlc");
case VLC_CACHE_DIR:
return config_GetHomeDir(".cache",
"/sdcard/Android/data/org.videolan.vlc/cache");
case VLC_HOME_DIR:
return config_GetHomeDir(NULL, NULL);
case VLC_CONFIG_DIR:
return config_GetHomeDir(".config", NULL);

case VLC_DESKTOP_DIR:
case VLC_DOWNLOAD_DIR:
case VLC_TEMPLATES_DIR:
case VLC_PUBLICSHARE_DIR:
case VLC_DOCUMENTS_DIR:
case VLC_MUSIC_DIR:
case VLC_PICTURES_DIR:
case VLC_VIDEOS_DIR:
{
assert(type >= VLC_DESKTOP_DIR && type <= VLC_VIDEOS_DIR);
const char *psz_name = ppsz_generic_names[type - VLC_DESKTOP_DIR];
if (psz_name != NULL)
return config_GetGenericDir(psz_name);
}
}
return NULL;
}

char *config_GetSysPath(vlc_sysdir_t type, const char *filename)
{
char *dir = NULL;

switch (type)
{
case VLC_SYSDATA_DIR:
dir = strdup("/system/usr/share");
break;
case VLC_LIB_DIR:
dir = config_GetLibDir();
break;
default:
break;
}

if (filename == NULL || dir == NULL)
return dir;

char *path;
if (unlikely(asprintf(&path, "%s/%s", dir, filename) == -1))
path = NULL;
free(dir);
return path;
}











char *vlc_getProxyUrl(const char *url)
{
VLC_UNUSED(url);
JNIEnv *env;
bool b_detach;
char *psz_ret = NULL;
const char *psz_host = NULL, *psz_port = NULL;
jstring jhost = NULL, jport = NULL;

env = get_env(&b_detach);
if (env == NULL)
return NULL;


jstring jkey = (*env)->NewStringUTF(env, "http.proxyHost");
if ((*env)->ExceptionCheck(env))
{
(*env)->ExceptionClear(env);
jkey = NULL;
}
if (jkey == NULL)
goto end;

jhost = (*env)->CallStaticObjectMethod(env, fields.System.clazz,
fields.System.getProperty, jkey);
(*env)->DeleteLocalRef(env, jkey);
if (jhost == NULL)
goto end;

psz_host = (*env)->GetStringUTFChars(env, jhost, 0);

if (psz_host == NULL || psz_host[0] == '\0')
goto end;


jkey = (*env)->NewStringUTF(env, "http.proxyPort");
if ((*env)->ExceptionCheck(env))
{
(*env)->ExceptionClear(env);
jkey = NULL;
}
if (jkey == NULL)
goto end;

jport = (*env)->CallStaticObjectMethod(env, fields.System.clazz,
fields.System.getProperty, jkey);
(*env)->DeleteLocalRef(env, jkey);


if (jport != NULL)
{
psz_port = (*env)->GetStringUTFChars(env, jport, 0);
if (psz_port != NULL && (psz_port[0] == '\0' || psz_port[0] == '0'))
{
(*env)->ReleaseStringUTFChars(env, jport, psz_port);
psz_port = NULL;
}
}


if (asprintf(&psz_ret, "http://%s%s%s",
psz_host,
psz_port != NULL ? ":" : "",
psz_port != NULL ? psz_port : "") == -1)
psz_ret = NULL;

end:
if (psz_host != NULL)
(*env)->ReleaseStringUTFChars(env, jhost, psz_host);
if (jhost != NULL)
(*env)->DeleteLocalRef(env, jhost);
if (psz_port != NULL)
(*env)->ReleaseStringUTFChars(env, jport, psz_port);
if (jport != NULL)
(*env)->DeleteLocalRef(env, jport);
release_env(b_detach);
return psz_ret;
}
