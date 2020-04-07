#define ALLEGRO_NO_COMPATIBILITY
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_exitfunc.h"
#include "allegro5/internal/aintern_mouse.h"
#include "allegro5/internal/aintern_system.h"
static ALLEGRO_MOUSE_DRIVER *new_mouse_driver = NULL;
bool al_is_mouse_installed(void)
{
return (new_mouse_driver ? true : false);
}
bool al_install_mouse(void)
{
if (new_mouse_driver)
return true;
if (al_get_system_driver()->vt->get_mouse_driver) {
new_mouse_driver = al_get_system_driver()->vt->get_mouse_driver();
if (!new_mouse_driver->init_mouse()) {
new_mouse_driver = NULL;
return false;
}
_al_add_exit_func(al_uninstall_mouse, "al_uninstall_mouse");
return true;
}
return false;
#if 0
if (system_driver && system_driver->mouse_drivers)
driver_list = system_driver->mouse_drivers();
else
driver_list = _al_mouse_driver_list;
ASSERT(driver_list);
for (i=0; driver_list[i].driver; i++) {
new_mouse_driver = driver_list[i].driver;
name = new_mouse_driver->msedrv_ascii_name;
new_mouse_driver->msedrv_name = name;
new_mouse_driver->msedrv_desc = name;
if (new_mouse_driver->init_mouse()) {
break;
}
}
if (!driver_list[i].driver) {
new_mouse_driver = NULL;
return false;
}
_al_add_exit_func(al_uninstall_mouse, "al_uninstall_mouse");
return true;
#endif
}
void al_uninstall_mouse(void)
{
if (!new_mouse_driver)
return;
new_mouse_driver->exit_mouse();
new_mouse_driver = NULL;
}
static ALLEGRO_MOUSE *al_get_mouse(void)
{
ALLEGRO_MOUSE *mse;
ASSERT(new_mouse_driver);
mse = new_mouse_driver->get_mouse();
ASSERT(mse);
return mse;
}
unsigned int al_get_mouse_num_buttons(void)
{
ASSERT(new_mouse_driver);
return new_mouse_driver->get_mouse_num_buttons();
}
unsigned int al_get_mouse_num_axes(void)
{
ASSERT(new_mouse_driver);
return new_mouse_driver->get_mouse_num_axes();
}
bool al_set_mouse_xy(ALLEGRO_DISPLAY *display, int x, int y)
{
ASSERT(new_mouse_driver);
ASSERT(new_mouse_driver->set_mouse_xy);
return new_mouse_driver->set_mouse_xy(display, x, y);
}
bool al_set_mouse_z(int z)
{
ASSERT(new_mouse_driver);
ASSERT(new_mouse_driver->set_mouse_axis);
return new_mouse_driver->set_mouse_axis(2, z);
}
bool al_set_mouse_w(int w)
{
ASSERT(new_mouse_driver);
ASSERT(new_mouse_driver->set_mouse_axis);
return new_mouse_driver->set_mouse_axis(3, w);
}
bool al_set_mouse_axis(int which, int value)
{
ASSERT(new_mouse_driver);
ASSERT(new_mouse_driver->set_mouse_axis);
ASSERT(which >= 2);
ASSERT(which < 4 + ALLEGRO_MOUSE_MAX_EXTRA_AXES);
if (which >= 2 && which < 4 + ALLEGRO_MOUSE_MAX_EXTRA_AXES)
return new_mouse_driver->set_mouse_axis(which, value);
else
return false;
}
void al_get_mouse_state(ALLEGRO_MOUSE_STATE *ret_state)
{
ASSERT(new_mouse_driver);
ASSERT(ret_state);
new_mouse_driver->get_mouse_state(ret_state);
}
int al_get_mouse_state_axis(const ALLEGRO_MOUSE_STATE *state, int axis)
{
ASSERT(state);
ASSERT(axis >= 0);
ASSERT(axis < (4 + ALLEGRO_MOUSE_MAX_EXTRA_AXES));
switch (axis) {
case 0:
return state->x;
case 1:
return state->y;
case 2:
return state->z;
case 3:
return state->w;
default:
return state->more_axes[axis - 4];
}
}
bool al_mouse_button_down(const ALLEGRO_MOUSE_STATE *state, int button)
{
ASSERT(state);
ASSERT(button > 0);
return (state->buttons & (1 << (button-1)));
}
bool al_get_mouse_cursor_position(int *ret_x, int *ret_y)
{
ALLEGRO_SYSTEM *alsys = al_get_system_driver();
ASSERT(ret_x);
ASSERT(ret_y);
if (alsys->vt->get_cursor_position) {
return alsys->vt->get_cursor_position(ret_x, ret_y);
}
else {
*ret_x = 0;
*ret_y = 0;
return false;
}
}
bool al_grab_mouse(ALLEGRO_DISPLAY *display)
{
ALLEGRO_SYSTEM *alsys = al_get_system_driver();
if (alsys->vt->grab_mouse)
return alsys->vt->grab_mouse(display);
return false;
}
bool al_ungrab_mouse(void)
{
ALLEGRO_SYSTEM *alsys = al_get_system_driver();
if (alsys->vt->ungrab_mouse)
return alsys->vt->ungrab_mouse();
return false;
}
ALLEGRO_EVENT_SOURCE *al_get_mouse_event_source(void)
{
ALLEGRO_MOUSE *mouse = al_get_mouse();
return (mouse) ? &mouse->es : NULL;
}
void al_set_mouse_wheel_precision(int precision)
{
ALLEGRO_SYSTEM *alsys = al_get_system_driver();
ASSERT(alsys);
if (precision < 1)
precision = 1;
alsys->mouse_wheel_precision = precision;
}
int al_get_mouse_wheel_precision(void)
{
ALLEGRO_SYSTEM *alsys = al_get_system_driver();
ASSERT(alsys);
return alsys->mouse_wheel_precision;
}
