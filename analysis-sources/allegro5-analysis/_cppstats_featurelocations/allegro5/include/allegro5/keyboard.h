














#if !defined(__al_included_allegro5_keyboard_h)
#define __al_included_allegro5_keyboard_h

#include "allegro5/base.h"
#include "allegro5/events.h"
#include "allegro5/keycodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct ALLEGRO_KEYBOARD ALLEGRO_KEYBOARD;





typedef struct ALLEGRO_KEYBOARD_STATE ALLEGRO_KEYBOARD_STATE;

struct ALLEGRO_KEYBOARD_STATE
{
struct ALLEGRO_DISPLAY *display; 

unsigned int __key_down__internal__[(ALLEGRO_KEY_MAX + 31) / 32];
};


AL_FUNC(bool, al_is_keyboard_installed, (void));
AL_FUNC(bool, al_install_keyboard, (void));
AL_FUNC(void, al_uninstall_keyboard, (void));

AL_FUNC(bool, al_set_keyboard_leds, (int leds));

AL_FUNC(const char *, al_keycode_to_name, (int keycode));

AL_FUNC(void, al_get_keyboard_state, (ALLEGRO_KEYBOARD_STATE *ret_state));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
AL_FUNC(void, al_clear_keyboard_state, (ALLEGRO_DISPLAY *display));
#endif
AL_FUNC(bool, al_key_down, (const ALLEGRO_KEYBOARD_STATE *, int keycode));

AL_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_keyboard_event_source, (void));

#if defined(__cplusplus)
}
#endif

#endif







