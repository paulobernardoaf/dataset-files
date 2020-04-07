#include "allegro5/allegro.h"
#include "allegro5/allegro_android.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_android.h"
#include "allegro5/internal/aintern_bitmap.h"
#include <jni.h>
ALLEGRO_DEBUG_CHANNEL("clipboard")
static bool android_set_clipboard_text(ALLEGRO_DISPLAY *display, const char *text)
{
JNIEnv * env = (JNIEnv *)_al_android_get_jnienv();
jstring jtext= _jni_call(env, jstring, NewStringUTF, text);
(void) display;
return _jni_callBooleanMethodV(env, _al_android_activity_object(),
"setClipboardText", "(Ljava/lang/String;)Z", jtext);
}
static char *android_get_clipboard_text(ALLEGRO_DISPLAY *display)
{
JNIEnv * env = (JNIEnv *)_al_android_get_jnienv();
jobject jtext = _jni_callObjectMethod(env, _al_android_activity_object(), "getClipboardText", "()Ljava/lang/String;");
jsize len = _jni_call(env, jsize, GetStringUTFLength, jtext);
const char *str = _jni_call(env, const char *, GetStringUTFChars, jtext, NULL);
char * text = al_malloc(len+1);
(void) display;
text = _al_sane_strncpy(text, str, len);
_jni_callv(env, ReleaseStringUTFChars, jtext, str);
_jni_callv(env, DeleteLocalRef, jtext);
return text;
}
static bool android_has_clipboard_text(ALLEGRO_DISPLAY *display)
{
JNIEnv * env = (JNIEnv *)_al_android_get_jnienv();
(void) display;
return _jni_callBooleanMethodV(env, _al_android_activity_object(),
"hasClipboardText", "()Z");
}
void _al_android_add_clipboard_functions(ALLEGRO_DISPLAY_INTERFACE *vt)
{
vt->set_clipboard_text = android_set_clipboard_text;
vt->get_clipboard_text = android_get_clipboard_text;
vt->has_clipboard_text = android_has_clipboard_text;
}
