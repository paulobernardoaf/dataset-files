#include "allegro5/base.h"
#include "allegro5/events.h"
#if defined(__cplusplus)
extern "C" {
#endif
#define ALLEGRO_MOUSE_MAX_EXTRA_AXES 4
typedef struct ALLEGRO_MOUSE ALLEGRO_MOUSE;
typedef struct ALLEGRO_MOUSE_STATE ALLEGRO_MOUSE_STATE;
struct ALLEGRO_MOUSE_STATE
{
int x;
int y;
int z;
int w;
int more_axes[ALLEGRO_MOUSE_MAX_EXTRA_AXES];
int buttons;
float pressure;
struct ALLEGRO_DISPLAY *display;
};
AL_FUNC(bool, al_is_mouse_installed, (void));
AL_FUNC(bool, al_install_mouse, (void));
AL_FUNC(void, al_uninstall_mouse, (void));
AL_FUNC(unsigned int, al_get_mouse_num_buttons, (void));
AL_FUNC(unsigned int, al_get_mouse_num_axes, (void));
AL_FUNC(bool, al_set_mouse_xy, (struct ALLEGRO_DISPLAY *display, int x, int y));
AL_FUNC(bool, al_set_mouse_z, (int z));
AL_FUNC(bool, al_set_mouse_w, (int w));
AL_FUNC(bool, al_set_mouse_axis, (int axis, int value));
AL_FUNC(void, al_get_mouse_state, (ALLEGRO_MOUSE_STATE *ret_state));
AL_FUNC(bool, al_mouse_button_down, (const ALLEGRO_MOUSE_STATE *state, int button));
AL_FUNC(int, al_get_mouse_state_axis, (const ALLEGRO_MOUSE_STATE *state, int axis));
AL_FUNC(bool, al_get_mouse_cursor_position, (int *ret_x, int *ret_y));
AL_FUNC(bool, al_grab_mouse, (struct ALLEGRO_DISPLAY *display));
AL_FUNC(bool, al_ungrab_mouse, (void));
AL_FUNC(void, al_set_mouse_wheel_precision, (int precision));
AL_FUNC(int, al_get_mouse_wheel_precision, (void));
AL_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_mouse_event_source, (void));
#if defined(__cplusplus)
}
#endif
