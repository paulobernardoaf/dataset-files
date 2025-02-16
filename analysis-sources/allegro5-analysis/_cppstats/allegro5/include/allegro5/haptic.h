#include "allegro5/base.h"
#include "allegro5/display.h"
#include "allegro5/events.h"
#include "allegro5/joystick.h"
#include "allegro5/keyboard.h"
#include "allegro5/mouse.h"
#include "allegro5/touch_input.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
enum ALLEGRO_HAPTIC_CONSTANTS
{
ALLEGRO_HAPTIC_RUMBLE = 1 << 0,
ALLEGRO_HAPTIC_PERIODIC = 1 << 1,
ALLEGRO_HAPTIC_CONSTANT = 1 << 2,
ALLEGRO_HAPTIC_SPRING = 1 << 3,
ALLEGRO_HAPTIC_FRICTION = 1 << 4,
ALLEGRO_HAPTIC_DAMPER = 1 << 5,
ALLEGRO_HAPTIC_INERTIA = 1 << 6,
ALLEGRO_HAPTIC_RAMP = 1 << 7,
ALLEGRO_HAPTIC_SQUARE = 1 << 8,
ALLEGRO_HAPTIC_TRIANGLE = 1 << 9,
ALLEGRO_HAPTIC_SINE = 1 << 10,
ALLEGRO_HAPTIC_SAW_UP = 1 << 11,
ALLEGRO_HAPTIC_SAW_DOWN = 1 << 12,
ALLEGRO_HAPTIC_CUSTOM = 1 << 13,
ALLEGRO_HAPTIC_GAIN = 1 << 14,
ALLEGRO_HAPTIC_ANGLE = 1 << 15,
ALLEGRO_HAPTIC_RADIUS = 1 << 16,
ALLEGRO_HAPTIC_AZIMUTH = 1 << 17,
ALLEGRO_HAPTIC_AUTOCENTER= 1 << 18,
};
typedef struct ALLEGRO_HAPTIC ALLEGRO_HAPTIC;
struct ALLEGRO_HAPTIC_DIRECTION
{
double angle;
double radius;
double azimuth;
};
struct ALLEGRO_HAPTIC_REPLAY
{
double length;
double delay;
};
struct ALLEGRO_HAPTIC_ENVELOPE
{
double attack_length;
double attack_level;
double fade_length;
double fade_level;
};
struct ALLEGRO_HAPTIC_CONSTANT_EFFECT
{
double level;
struct ALLEGRO_HAPTIC_ENVELOPE envelope;
};
struct ALLEGRO_HAPTIC_RAMP_EFFECT
{
double start_level;
double end_level;
struct ALLEGRO_HAPTIC_ENVELOPE envelope;
};
struct ALLEGRO_HAPTIC_CONDITION_EFFECT
{
double right_saturation;
double left_saturation;
double right_coeff;
double left_coeff;
double deadband;
double center;
};
struct ALLEGRO_HAPTIC_PERIODIC_EFFECT
{
int waveform;
double period;
double magnitude;
double offset;
double phase;
struct ALLEGRO_HAPTIC_ENVELOPE envelope;
int custom_len;
double *custom_data;
};
struct ALLEGRO_HAPTIC_RUMBLE_EFFECT
{
double strong_magnitude;
double weak_magnitude;
};
union ALLEGRO_HAPTIC_EFFECT_UNION
{
struct ALLEGRO_HAPTIC_CONSTANT_EFFECT constant;
struct ALLEGRO_HAPTIC_RAMP_EFFECT ramp;
struct ALLEGRO_HAPTIC_PERIODIC_EFFECT periodic;
struct ALLEGRO_HAPTIC_CONDITION_EFFECT condition;
struct ALLEGRO_HAPTIC_RUMBLE_EFFECT rumble;
};
struct ALLEGRO_HAPTIC_EFFECT
{
int type;
struct ALLEGRO_HAPTIC_DIRECTION direction;
struct ALLEGRO_HAPTIC_REPLAY replay;
union ALLEGRO_HAPTIC_EFFECT_UNION data;
};
typedef struct ALLEGRO_HAPTIC_EFFECT ALLEGRO_HAPTIC_EFFECT;
typedef struct ALLEGRO_HAPTIC_EFFECT_ID ALLEGRO_HAPTIC_EFFECT_ID;
struct ALLEGRO_HAPTIC_EFFECT_ID
{
ALLEGRO_HAPTIC *_haptic;
int _id;
int _handle;
void * _pointer;
double _effect_duration;
bool _playing;
double _start_time;
double _end_time;
void * driver;
};
AL_FUNC(bool, al_install_haptic, (void));
AL_FUNC(void, al_uninstall_haptic, (void));
AL_FUNC(bool, al_is_haptic_installed, (void));
AL_FUNC(bool, al_is_mouse_haptic, (ALLEGRO_MOUSE *));
AL_FUNC(bool, al_is_joystick_haptic, (ALLEGRO_JOYSTICK *));
AL_FUNC(bool, al_is_keyboard_haptic, (ALLEGRO_KEYBOARD *));
AL_FUNC(bool, al_is_display_haptic, (ALLEGRO_DISPLAY *));
AL_FUNC(bool, al_is_touch_input_haptic, (ALLEGRO_TOUCH_INPUT *));
AL_FUNC(ALLEGRO_HAPTIC *, al_get_haptic_from_mouse, (ALLEGRO_MOUSE *));
AL_FUNC(ALLEGRO_HAPTIC *, al_get_haptic_from_joystick, (ALLEGRO_JOYSTICK *));
AL_FUNC(ALLEGRO_HAPTIC *, al_get_haptic_from_keyboard, (ALLEGRO_KEYBOARD *));
AL_FUNC(ALLEGRO_HAPTIC *, al_get_haptic_from_display, (ALLEGRO_DISPLAY *));
AL_FUNC(ALLEGRO_HAPTIC *, al_get_haptic_from_touch_input, (ALLEGRO_TOUCH_INPUT *));
AL_FUNC(bool, al_release_haptic, (ALLEGRO_HAPTIC *));
AL_FUNC(bool, al_is_haptic_active, (ALLEGRO_HAPTIC *));
AL_FUNC(int, al_get_haptic_capabilities, (ALLEGRO_HAPTIC *));
AL_FUNC(bool, al_is_haptic_capable, (ALLEGRO_HAPTIC *, int));
AL_FUNC(bool, al_set_haptic_gain, (ALLEGRO_HAPTIC *, double));
AL_FUNC(double, al_get_haptic_gain, (ALLEGRO_HAPTIC *));
AL_FUNC(bool, al_set_haptic_autocenter, (ALLEGRO_HAPTIC *, double));
AL_FUNC(double, al_get_haptic_autocenter, (ALLEGRO_HAPTIC *));
AL_FUNC(int, al_get_max_haptic_effects, (ALLEGRO_HAPTIC *));
AL_FUNC(bool, al_is_haptic_effect_ok, (ALLEGRO_HAPTIC *, ALLEGRO_HAPTIC_EFFECT *));
AL_FUNC(bool, al_upload_haptic_effect, (ALLEGRO_HAPTIC *, ALLEGRO_HAPTIC_EFFECT *, ALLEGRO_HAPTIC_EFFECT_ID *));
AL_FUNC(bool, al_play_haptic_effect, (ALLEGRO_HAPTIC_EFFECT_ID *, int));
AL_FUNC(bool, al_upload_and_play_haptic_effect, (ALLEGRO_HAPTIC *, ALLEGRO_HAPTIC_EFFECT *, ALLEGRO_HAPTIC_EFFECT_ID *, int));
AL_FUNC(bool, al_stop_haptic_effect, (ALLEGRO_HAPTIC_EFFECT_ID *));
AL_FUNC(bool, al_is_haptic_effect_playing, (ALLEGRO_HAPTIC_EFFECT_ID *));
AL_FUNC(bool, al_release_haptic_effect, (ALLEGRO_HAPTIC_EFFECT_ID *));
AL_FUNC(double, al_get_haptic_effect_duration, (ALLEGRO_HAPTIC_EFFECT *));
AL_FUNC(bool, al_rumble_haptic, (ALLEGRO_HAPTIC *, double, double, ALLEGRO_HAPTIC_EFFECT_ID *));
#endif
#if defined(__cplusplus)
}
#endif
