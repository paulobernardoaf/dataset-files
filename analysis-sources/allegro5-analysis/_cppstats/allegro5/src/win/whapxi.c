#define ALLEGRO_NO_COMPATIBILITY
#define DIRECTINPUT_VERSION 0x0800
#define _WIN32_WINNT 0x0501
#include "allegro5/allegro.h"
#include "allegro5/haptic.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintwin.h"
#include "allegro5/internal/aintern_haptic.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_joystick.h"
#include "allegro5/internal/aintern_bitmap.h"
#if defined(ALLEGRO_CFG_XINPUT)
#if !defined(ALLEGRO_WINDOWS)
#error something is wrong with the makefile
#endif
#if !defined(ALLEGRO_XINPUT_POLL_DELAY)
#define ALLEGRO_XINPUT_POLL_DELAY 0.1
#endif
#if defined(ALLEGRO_MINGW32)
#undef MAKEFOURCC
#endif
#include <initguid.h>
#include <stdio.h>
#include <mmsystem.h>
#include <process.h>
#include <math.h>
#if defined(ALLEGRO_HAVE_SAL_H)
#include <sal.h>
#endif
#include <xinput.h>
#include "allegro5/internal/aintern_wjoyxi.h"
ALLEGRO_DEBUG_CHANNEL("haptic")
#define HAPTICS_MAX 4
#define HAPTIC_EFFECTS_MAX 1
typedef enum ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE
{
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE = 0,
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY = 1,
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STARTING = 2,
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_PLAYING = 3,
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_DELAYED = 4,
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STOPPING = 5,
} ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE;
typedef struct ALLEGRO_HAPTIC_EFFECT_XINPUT
{
ALLEGRO_HAPTIC_EFFECT effect;
XINPUT_VIBRATION vibration;
int id;
double start_time;
double loop_start;
double stop_time;
int repeats;
int delay_repeated;
int play_repeated;
ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE state;
} ALLEGRO_HAPTIC_EFFECT_XINPUT;
typedef struct ALLEGRO_HAPTIC_XINPUT
{
ALLEGRO_HAPTIC parent;
ALLEGRO_JOYSTICK_XINPUT *xjoy;
bool active;
ALLEGRO_HAPTIC_EFFECT_XINPUT effect;
int flags;
} ALLEGRO_HAPTIC_XINPUT;
static bool hapxi_init_haptic(void);
static void hapxi_exit_haptic(void);
static bool hapxi_is_mouse_haptic(ALLEGRO_MOUSE *dev);
static bool hapxi_is_joystick_haptic(ALLEGRO_JOYSTICK *);
static bool hapxi_is_keyboard_haptic(ALLEGRO_KEYBOARD *dev);
static bool hapxi_is_display_haptic(ALLEGRO_DISPLAY *dev);
static bool hapxi_is_touch_input_haptic(ALLEGRO_TOUCH_INPUT *dev);
static ALLEGRO_HAPTIC *hapxi_get_from_mouse(ALLEGRO_MOUSE *dev);
static ALLEGRO_HAPTIC *hapxi_get_from_joystick(ALLEGRO_JOYSTICK *dev);
static ALLEGRO_HAPTIC *hapxi_get_from_keyboard(ALLEGRO_KEYBOARD *dev);
static ALLEGRO_HAPTIC *hapxi_get_from_display(ALLEGRO_DISPLAY *dev);
static ALLEGRO_HAPTIC *hapxi_get_from_touch_input(ALLEGRO_TOUCH_INPUT *dev);
static bool hapxi_release(ALLEGRO_HAPTIC *haptic);
static bool hapxi_get_active(ALLEGRO_HAPTIC *hap);
static int hapxi_get_capabilities(ALLEGRO_HAPTIC *dev);
static double hapxi_get_gain(ALLEGRO_HAPTIC *dev);
static bool hapxi_set_gain(ALLEGRO_HAPTIC *dev, double);
static int hapxi_get_max_effects(ALLEGRO_HAPTIC *dev);
static bool hapxi_is_effect_ok(ALLEGRO_HAPTIC *dev, ALLEGRO_HAPTIC_EFFECT *eff);
static bool hapxi_upload_effect(ALLEGRO_HAPTIC *dev,
ALLEGRO_HAPTIC_EFFECT *eff,
ALLEGRO_HAPTIC_EFFECT_ID *id);
static bool hapxi_play_effect(ALLEGRO_HAPTIC_EFFECT_ID *id, int loop);
static bool hapxi_stop_effect(ALLEGRO_HAPTIC_EFFECT_ID *id);
static bool hapxi_is_effect_playing(ALLEGRO_HAPTIC_EFFECT_ID *id);
static bool hapxi_release_effect(ALLEGRO_HAPTIC_EFFECT_ID *id);
static double hapxi_get_autocenter(ALLEGRO_HAPTIC *dev);
static bool hapxi_set_autocenter(ALLEGRO_HAPTIC *dev, double);
static void *hapxi_poll_thread(ALLEGRO_THREAD *thread, void *arg);
ALLEGRO_HAPTIC_DRIVER _al_hapdrv_xinput =
{
AL_HAPTIC_TYPE_XINPUT,
"",
"",
"Windows XInput haptic(s)",
hapxi_init_haptic,
hapxi_exit_haptic,
hapxi_is_mouse_haptic,
hapxi_is_joystick_haptic,
hapxi_is_keyboard_haptic,
hapxi_is_display_haptic,
hapxi_is_touch_input_haptic,
hapxi_get_from_mouse,
hapxi_get_from_joystick,
hapxi_get_from_keyboard,
hapxi_get_from_display,
hapxi_get_from_touch_input,
hapxi_get_active,
hapxi_get_capabilities,
hapxi_get_gain,
hapxi_set_gain,
hapxi_get_max_effects,
hapxi_is_effect_ok,
hapxi_upload_effect,
hapxi_play_effect,
hapxi_stop_effect,
hapxi_is_effect_playing,
hapxi_release_effect,
hapxi_release,
hapxi_get_autocenter,
hapxi_set_autocenter
};
static ALLEGRO_HAPTIC_XINPUT haptics[HAPTICS_MAX];
static ALLEGRO_THREAD *hapxi_thread = NULL;
static ALLEGRO_MUTEX *hapxi_mutex = NULL;
static ALLEGRO_COND *hapxi_cond = NULL;
typedef DWORD(WINAPI *XInputSetStatePROC)(DWORD, XINPUT_VIBRATION*);
extern XInputSetStatePROC _al_imp_XInputSetState;
static bool hapxi_force_stop(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
XINPUT_VIBRATION no_vibration = { 0, 0 };
ALLEGRO_DEBUG("XInput haptic effect stopped.\n");
_al_imp_XInputSetState(hapxi->xjoy->index, &no_vibration);
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY;
return true;
}
static bool hapxi_force_play(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
DWORD res;
res = _al_imp_XInputSetState(hapxi->xjoy->index, &effxi->vibration);
ALLEGRO_DEBUG("Starting to play back haptic effect: %d.\n", (int)(res));
if (res == ERROR_SUCCESS) {
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_PLAYING;
return true;
}
else {
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY;
return false;
}
}
static bool hapxi_poll_haptic_effect_ready(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
(void)hapxi; (void)effxi;
return true;
}
static bool hapxi_poll_haptic_effect_starting(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
double now = al_get_time();
if ((now - effxi->start_time) < effxi->effect.replay.delay) {
effxi->loop_start = al_get_time();
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_DELAYED;
}
else {
hapxi_force_play(hapxi, effxi);
}
ALLEGRO_DEBUG("Polling XInput haptic effect. Really Starting: %d!\n", effxi->state);
return true;
}
static bool hapxi_poll_haptic_effect_playing(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
double now = al_get_time();
double stop = effxi->loop_start + effxi->effect.replay.delay +
effxi->effect.replay.length;
(void)hapxi;
if (now > stop) {
effxi->play_repeated++;
if (effxi->play_repeated < effxi->repeats) {
hapxi_force_stop(hapxi, effxi);
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_DELAYED;
effxi->loop_start = al_get_time();
}
else {
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STOPPING;
}
return true;
}
return false;
}
static bool hapxi_poll_haptic_effect_delayed(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
double now = al_get_time();
if (now > (effxi->loop_start + effxi->effect.replay.delay)) {
return hapxi_force_play(hapxi, effxi);
}
return false;
}
static bool hapxi_poll_haptic_effect_stopping(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
return hapxi_force_stop(hapxi, effxi);
}
static bool hapxi_poll_haptic_effect(ALLEGRO_HAPTIC_XINPUT *hapxi,
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
switch (effxi->state) {
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE:
return false;
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY:
return hapxi_poll_haptic_effect_ready(hapxi, effxi);
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STARTING:
return hapxi_poll_haptic_effect_starting(hapxi, effxi);
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_PLAYING:
return hapxi_poll_haptic_effect_playing(hapxi, effxi);
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_DELAYED:
return hapxi_poll_haptic_effect_delayed(hapxi, effxi);
case ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STOPPING:
return hapxi_poll_haptic_effect_stopping(hapxi, effxi);
default:
ALLEGRO_DEBUG("XInput haptic effect state not valid :%d.\n", effxi->state);
return false;
}
}
static void hapxi_poll_haptic(ALLEGRO_HAPTIC_XINPUT *hapxi)
{
hapxi_poll_haptic_effect(hapxi, &hapxi->effect);
}
static void hapxi_poll_haptics(void)
{
int i;
for (i = 0; i < HAPTICS_MAX; i++) {
if (haptics[i].active) {
hapxi_poll_haptic(haptics + i);
}
}
}
static void *hapxi_poll_thread(ALLEGRO_THREAD *thread, void *arg)
{
ALLEGRO_TIMEOUT timeout;
al_lock_mutex(hapxi_mutex);
while (!al_get_thread_should_stop(thread)) {
al_init_timeout(&timeout, ALLEGRO_XINPUT_POLL_DELAY);
al_wait_cond_until(hapxi_cond, hapxi_mutex, &timeout);
hapxi_poll_haptics();
}
al_unlock_mutex(hapxi_mutex);
return arg;
}
static bool hapxi_init_haptic(void)
{
int i;
ASSERT(hapxi_mutex == NULL);
ASSERT(hapxi_thread == NULL);
ASSERT(hapxi_cond == NULL);
hapxi_mutex = al_create_mutex_recursive();
if (!hapxi_mutex)
return false;
hapxi_cond = al_create_cond();
if (!hapxi_cond)
return false;
al_lock_mutex(hapxi_mutex);
for (i = 0; i < HAPTICS_MAX; i++) {
haptics[i].active = false;
}
hapxi_thread = al_create_thread(hapxi_poll_thread, NULL);
al_unlock_mutex(hapxi_mutex);
if (hapxi_thread) al_start_thread(hapxi_thread);
return(hapxi_thread != NULL);
}
static ALLEGRO_HAPTIC_XINPUT *hapxi_from_al(ALLEGRO_HAPTIC *hap)
{
return (ALLEGRO_HAPTIC_XINPUT *)hap;
}
static void hapxi_exit_haptic(void)
{
void *ret_value;
ASSERT(hapxi_thread);
ASSERT(hapxi_mutex);
ASSERT(hapxi_cond);
al_set_thread_should_stop(hapxi_thread);
al_signal_cond(hapxi_cond);
al_join_thread(hapxi_thread, &ret_value);
al_destroy_thread(hapxi_thread);
al_destroy_cond(hapxi_cond);
al_destroy_mutex(hapxi_mutex);
hapxi_mutex = NULL;
}
static bool hapxi_magnitude2win(WORD *word, double value)
{
if (!word) return false;
(*word) = (WORD)(65535 * value);
return true;
}
static bool hapxi_effect2win(
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi,
ALLEGRO_HAPTIC_EFFECT *effect,
ALLEGRO_HAPTIC_XINPUT *hapxi)
{
(void)hapxi;
if (effect->type != ALLEGRO_HAPTIC_RUMBLE)
return false;
return
hapxi_magnitude2win(&effxi->vibration.wRightMotorSpeed,
effect->data.rumble.weak_magnitude) &&
hapxi_magnitude2win(&effxi->vibration.wLeftMotorSpeed,
effect->data.rumble.strong_magnitude);
}
static bool hapxi_get_active(ALLEGRO_HAPTIC *haptic)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_from_al(haptic);
return hapxi->active;
}
static bool hapxi_is_mouse_haptic(ALLEGRO_MOUSE *mouse)
{
(void)mouse;
return false;
}
static bool hapxi_is_joystick_haptic(ALLEGRO_JOYSTICK *joy)
{
ALLEGRO_JOYSTICK_XINPUT *joyxi = (ALLEGRO_JOYSTICK_XINPUT *)joy;
if (!al_is_joystick_installed())
return false;
if (!al_get_joystick_active(joy))
return false;
#if !defined(XINPUT_CAPS_FFB_SUPPORTED)
(void)joyxi;
ALLEGRO_DEBUG("Compiled against older XInput library, assuming force feedback support is available.\n");
return true;
#else
ALLEGRO_DEBUG("joyxi->capabilities.Flags: %d <-> %d\n", joyxi->capabilities.Flags, XINPUT_CAPS_FFB_SUPPORTED);
return(joyxi->capabilities.Flags & XINPUT_CAPS_FFB_SUPPORTED);
#endif
}
static bool hapxi_is_display_haptic(ALLEGRO_DISPLAY *dev)
{
(void)dev;
return false;
}
static bool hapxi_is_keyboard_haptic(ALLEGRO_KEYBOARD *dev)
{
(void)dev;
return false;
}
static bool hapxi_is_touch_input_haptic(ALLEGRO_TOUCH_INPUT *dev)
{
(void)dev;
return false;
}
static ALLEGRO_HAPTIC *hapxi_get_from_mouse(ALLEGRO_MOUSE *mouse)
{
(void)mouse;
return NULL;
}
static ALLEGRO_HAPTIC *hapxi_get_from_joystick(ALLEGRO_JOYSTICK *joy)
{
ALLEGRO_JOYSTICK_XINPUT *joyxi = (ALLEGRO_JOYSTICK_XINPUT *)joy;
ALLEGRO_HAPTIC_XINPUT *hapxi;
if (!al_is_joystick_haptic(joy))
return NULL;
al_lock_mutex(hapxi_mutex);
hapxi = haptics + joyxi->index;
hapxi->parent.driver = &_al_hapdrv_xinput;
hapxi->parent.device = joyxi;
hapxi->parent.from = _AL_HAPTIC_FROM_JOYSTICK;
hapxi->active = true;
hapxi->effect.state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE;
hapxi->parent.gain = 1.0;
hapxi->parent.autocenter = 0.0;
hapxi->flags = ALLEGRO_HAPTIC_RUMBLE;
hapxi->xjoy = joyxi;
al_unlock_mutex(hapxi_mutex);
return &hapxi->parent;
}
static ALLEGRO_HAPTIC *hapxi_get_from_display(ALLEGRO_DISPLAY *dev)
{
(void)dev;
return NULL;
}
static ALLEGRO_HAPTIC *hapxi_get_from_keyboard(ALLEGRO_KEYBOARD *dev)
{
(void)dev;
return NULL;
}
static ALLEGRO_HAPTIC *hapxi_get_from_touch_input(ALLEGRO_TOUCH_INPUT *dev)
{
(void)dev;
return NULL;
}
static int hapxi_get_capabilities(ALLEGRO_HAPTIC *dev)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_from_al(dev);
return hapxi->flags;
}
static double hapxi_get_gain(ALLEGRO_HAPTIC *dev)
{
(void)dev;
return 1.0;
}
static bool hapxi_set_gain(ALLEGRO_HAPTIC *dev, double gain)
{
(void)dev; (void)gain;
return false;
}
double hapxi_get_autocenter(ALLEGRO_HAPTIC *dev)
{
(void)dev;
return 0.0;
}
static bool hapxi_set_autocenter(ALLEGRO_HAPTIC *dev, double intensity)
{
(void)dev; (void)intensity;
return false;
}
static int hapxi_get_max_effects(ALLEGRO_HAPTIC *dev)
{
(void)dev;
return 1;
}
static bool hapxi_is_effect_ok(ALLEGRO_HAPTIC *haptic,
ALLEGRO_HAPTIC_EFFECT *effect)
{
int caps;
caps = al_get_haptic_capabilities(haptic);
if (caps & effect->type) {
return true;
}
return false;
}
static ALLEGRO_HAPTIC_EFFECT_XINPUT *
hapxi_get_available_effect(ALLEGRO_HAPTIC_XINPUT *hapxi)
{
if (hapxi->effect.state == ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE) {
hapxi->effect.id = 0;
return &hapxi->effect;
}
return NULL;
}
static bool hapxi_release_effect_windows(ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi)
{
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE;
return true;
}
static bool hapxi_upload_effect(ALLEGRO_HAPTIC *dev,
ALLEGRO_HAPTIC_EFFECT *effect, ALLEGRO_HAPTIC_EFFECT_ID *id)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_from_al(dev);
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi = NULL;
ASSERT(dev);
ASSERT(id);
ASSERT(effect);
id->_haptic = NULL;
id->_id = -1;
id->_pointer = NULL;
id->_playing = false;
id->_effect_duration = 0.0;
id->_start_time = 0.0;
id->_end_time = 0.0;
if (!al_is_haptic_effect_ok(dev, effect))
return false;
al_lock_mutex(hapxi_mutex);
effxi = hapxi_get_available_effect(hapxi);
if (!effxi) {
ALLEGRO_WARN("No free effect slot.");
al_unlock_mutex(hapxi_mutex);
return false;
}
if (!hapxi_effect2win(effxi, effect, hapxi)) {
ALLEGRO_WARN("Cannot convert haptic effect to XINPUT effect.\n");
al_unlock_mutex(hapxi_mutex);
return false;
}
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY;
effxi->effect = (*effect);
id->_haptic = dev;
id->_pointer = effxi;
id->_id = effxi->id;
id->_effect_duration = al_get_haptic_effect_duration(effect);
al_unlock_mutex(hapxi_mutex);
return true;
}
static ALLEGRO_HAPTIC_XINPUT *
hapxi_device_for_id(ALLEGRO_HAPTIC_EFFECT_ID *id)
{
return (ALLEGRO_HAPTIC_XINPUT *)id->_haptic;
}
static ALLEGRO_HAPTIC_EFFECT_XINPUT *
hapxi_effect_for_id(ALLEGRO_HAPTIC_EFFECT_ID *id)
{
return (ALLEGRO_HAPTIC_EFFECT_XINPUT *)id->_pointer;
}
static bool hapxi_play_effect(ALLEGRO_HAPTIC_EFFECT_ID *id, int loops)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_device_for_id(id);
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi = hapxi_effect_for_id(id);
if ((!hapxi) || (id->_id < 0) || (!effxi) || (loops < 1))
return false;
al_lock_mutex(hapxi_mutex);
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STARTING;
effxi->start_time = al_get_time();
effxi->stop_time = effxi->start_time + al_get_haptic_effect_duration(&effxi->effect) * loops;
effxi->repeats = loops;
effxi->play_repeated = 0;
effxi->loop_start = effxi->start_time;
id->_playing = true;
id->_start_time = al_get_time();
id->_start_time = effxi->start_time;
id->_end_time = effxi->stop_time;
al_unlock_mutex(hapxi_mutex);
al_signal_cond(hapxi_cond);
return true;
}
static bool hapxi_stop_effect(ALLEGRO_HAPTIC_EFFECT_ID *id)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_device_for_id(id);
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi = hapxi_effect_for_id(id);
if ((!hapxi) || (id->_id < 0))
return false;
effxi = (ALLEGRO_HAPTIC_EFFECT_XINPUT *)id->_pointer;
if (effxi->state <= ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY) return false;
al_lock_mutex(hapxi_mutex);
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_STOPPING;
id->_playing = false;
al_unlock_mutex(hapxi_mutex);
al_signal_cond(hapxi_cond);
return true;
}
static bool hapxi_is_effect_playing(ALLEGRO_HAPTIC_EFFECT_ID *id)
{
ALLEGRO_HAPTIC_XINPUT *hapxi;
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi;
bool result;
ASSERT(id);
hapxi = hapxi_device_for_id(id);
effxi = hapxi_effect_for_id(id);
if ((!hapxi) || (id->_id < 0) || (!id->_playing))
return false;
al_lock_mutex(hapxi_mutex);
ALLEGRO_DEBUG("Playing effect state: %d %p %lf %lf\n", effxi->state, effxi, al_get_time(), id->_end_time);
result = (effxi->state > ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_READY);
al_unlock_mutex(hapxi_mutex);
al_signal_cond(hapxi_cond);
return result;
}
static bool hapxi_release_effect(ALLEGRO_HAPTIC_EFFECT_ID *id)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_device_for_id(id);
ALLEGRO_HAPTIC_EFFECT_XINPUT *effxi = hapxi_effect_for_id(id);
bool result;
if ((!hapxi) || (!effxi))
return false;
al_lock_mutex(hapxi_mutex);
hapxi_force_stop(hapxi, effxi);
effxi->state = ALLEGRO_HAPTIC_EFFECT_XINPUT_STATE_INACTIVE;
result = hapxi_release_effect_windows(effxi);
al_unlock_mutex(hapxi_mutex);
return result;
}
static bool hapxi_release(ALLEGRO_HAPTIC *haptic)
{
ALLEGRO_HAPTIC_XINPUT *hapxi = hapxi_from_al(haptic);
ASSERT(haptic);
if (!hapxi->active)
return false;
al_lock_mutex(hapxi_mutex);
hapxi_force_stop(hapxi, &hapxi->effect);
hapxi_release_effect_windows(&hapxi->effect);
hapxi->active = false;
hapxi->parent.device = NULL;
al_unlock_mutex(hapxi_mutex);
return true;
}
#endif
