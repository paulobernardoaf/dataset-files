#include "allegro5/base.h"
#include "allegro5/events.h"
#if defined(__cplusplus)
extern "C" {
#endif
#define ALLEGRO_TOUCH_INPUT_MAX_TOUCH_COUNT 16
typedef struct ALLEGRO_TOUCH_INPUT ALLEGRO_TOUCH_INPUT;
typedef struct ALLEGRO_TOUCH_INPUT_STATE ALLEGRO_TOUCH_INPUT_STATE;
typedef struct ALLEGRO_TOUCH_STATE ALLEGRO_TOUCH_STATE;
struct ALLEGRO_TOUCH_STATE
{
int id;
float x, y;
float dx, dy;
bool primary;
struct ALLEGRO_DISPLAY *display;
};
struct ALLEGRO_TOUCH_INPUT_STATE
{
ALLEGRO_TOUCH_STATE touches[ALLEGRO_TOUCH_INPUT_MAX_TOUCH_COUNT];
};
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
typedef enum ALLEGRO_MOUSE_EMULATION_MODE
{
ALLEGRO_MOUSE_EMULATION_NONE,
ALLEGRO_MOUSE_EMULATION_TRANSPARENT,
ALLEGRO_MOUSE_EMULATION_INCLUSIVE,
ALLEGRO_MOUSE_EMULATION_EXCLUSIVE,
ALLEGRO_MOUSE_EMULATION_5_0_x
} ALLEGRO_MOUSE_EMULATION_MODE;
#endif
AL_FUNC(bool, al_is_touch_input_installed, (void));
AL_FUNC(bool, al_install_touch_input, (void));
AL_FUNC(void, al_uninstall_touch_input, (void));
AL_FUNC(void, al_get_touch_input_state, (ALLEGRO_TOUCH_INPUT_STATE *ret_state));
AL_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_touch_input_event_source, (void));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
AL_FUNC(void, al_set_mouse_emulation_mode, (int mode));
AL_FUNC(int, al_get_mouse_emulation_mode, (void));
AL_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_touch_input_mouse_emulation_event_source, (void));
#endif
#if defined(__cplusplus)
}
#endif
