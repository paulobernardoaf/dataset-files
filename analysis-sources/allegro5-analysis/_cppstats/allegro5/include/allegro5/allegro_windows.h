#include <windows.h>
#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(HWND, al_get_win_window_handle, (ALLEGRO_DISPLAY *display));
AL_FUNC(bool, al_win_add_window_callback, (ALLEGRO_DISPLAY *display,
bool (*callback)(ALLEGRO_DISPLAY *display, UINT message, WPARAM wparam,
LPARAM lparam, LRESULT *result, void *userdata), void *userdata));
AL_FUNC(bool, al_win_remove_window_callback, (ALLEGRO_DISPLAY *display,
bool (*callback)(ALLEGRO_DISPLAY *display, UINT message, WPARAM wparam,
LPARAM lparam, LRESULT *result, void *userdata), void *userdata));
#if defined(__cplusplus)
}
#endif
