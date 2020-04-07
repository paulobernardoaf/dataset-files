#include <allegro5/allegro.h>
#if defined(ALLEGRO_ANDROID)
#include <allegro5/allegro_android.h>
#endif
#include "global.h"
#include "credits.h"
#include "fps.h"
#include "framework.h"
#include "game.h"
#include "gamepad.h"
#include "gamestate.h"
#include "keyboard.h"
#include "mouse.h"
#include "menus.h"
#include "screenshot.h"
#include "transition.h"
static int closed = false;
#define DEMO_CFG "demo.cfg"
static int timer = 0;
static SCREENSHOT *screenshot = NULL;
static int F12 = 0;
static FPS *fps = NULL;
static GAMESTATE state[DEMO_MAX_GAMESTATES];
static int current_state = 0, last_state;
static int state_count = 0;
ALLEGRO_EVENT_QUEUE *event_queue;
static TRANSITION *transition = NULL;
static void drop_build_config_dir(ALLEGRO_PATH *path)
{
const char *s = al_get_path_tail(path);
if (s) {
if (0 == strcmp(s, "Debug")
|| 0 == strcmp(s, "Release")
|| 0 == strcmp(s, "RelWithDebInfo")
|| 0 == strcmp(s, "Profile"))
{
al_drop_path_tail(path);
}
}
}
int init_framework(void)
{
int error = DEMO_OK;
int c;
ALLEGRO_PATH *path;
if (!al_init()) {
return DEMO_ERROR_ALLEGRO;
}
al_init_image_addon();
al_init_primitives_addon();
al_init_font_addon();
al_set_app_name("Allegro Skater Demo");
al_set_org_name("");
path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
al_make_directory(al_path_cstr(path, '/'));
al_set_path_filename(path, DEMO_CFG);
strncpy(config_path, al_path_cstr(path, '/'), DEMO_PATH_LENGTH);
al_destroy_path(path);
#if defined(ALLEGRO_ANDROID)
al_android_set_apk_file_interface();
strncpy(data_path, "/data", DEMO_PATH_LENGTH);
(void)drop_build_config_dir;
#else
path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
al_set_path_filename(path, "");
drop_build_config_dir(path);
al_append_path_component(path, "data");
strncpy(data_path, al_path_cstr(path, '/'), DEMO_PATH_LENGTH);
al_destroy_path(path);
#endif
read_global_config(config_path);
al_set_window_title(screen, "Allegro Demo Game");
al_install_audio();
al_init_acodec_addon();
al_reserve_samples(8);
event_queue = al_create_event_queue();
if ((error = change_gfx_mode()) != DEMO_OK) {
fprintf(stderr, "Error: %s\n", demo_error(error));
return error;
}
if (!al_install_keyboard()) {
fprintf(stderr, "Error installing keyboard: %s\n",
demo_error(DEMO_ERROR_ALLEGRO));
return DEMO_ERROR_ALLEGRO;
}
al_install_joystick();
if (al_install_touch_input())
al_register_event_source(event_queue, al_get_touch_input_event_source());
al_install_mouse();
al_register_event_source(event_queue, al_get_keyboard_event_source());
al_register_event_source(event_queue, al_get_mouse_event_source());
al_register_event_source(event_queue, al_get_joystick_event_source());
{
ALLEGRO_TIMER *t = al_create_timer(1.0 / logic_framerate);
al_register_event_source(event_queue, al_get_timer_event_source(t));
al_start_timer(t);
}
srand((unsigned)time(NULL));
screenshot = create_screenshot("SHOT", "TGA");
fps = create_fps(logic_framerate);
c = DEMO_MAX_GAMESTATES;
while (c--) {
state[c].deinit = state[c].init = NULL;
state[c].id = state[c].update = NULL;
state[c].draw = NULL;
}
create_main_menu(&state[0]);
create_options_menu(&state[1]);
create_gfx_menu(&state[2]);
create_sound_menu(&state[3]);
create_misc_menu(&state[4]);
create_controls_menu(&state[5]);
create_intro(&state[6]);
create_about_menu(&state[7]);
create_new_game(&state[8]);
create_continue_game(&state[9]);
create_success_menu(&state[10]);
state_count = 11; 
current_state = 6; 
controller[DEMO_CONTROLLER_KEYBOARD] =
create_keyboard_controller(config_path);
controller[DEMO_CONTROLLER_GAMEPAD] =
create_gamepad_controller(config_path);
init_credits();
return error;
}
static void draw_framework(void)
{
if (transition) {
draw_transition(transition);
} else {
if (current_state != DEMO_STATE_EXIT) {
state[current_state].draw();
}
}
if (display_framerate) {
draw_fps(fps, plain_font, screen_width, 0, al_map_rgb(255, 255, 255), "%d FPS");
}
al_flip_display();
}
void run_framework(void)
{
int done = false; 
int need_to_redraw = 1; 
int next_state = current_state;
int i;
bool background_mode = false;
bool paused = false;
state[current_state].init();
transition = create_transition(NULL, &state[current_state], 0.3f);
timer = 0;
while (!done) {
ALLEGRO_EVENT event;
al_wait_for_event(event_queue, &event);
switch (event.type) {
case ALLEGRO_EVENT_MOUSE_AXES:
mouse_handle_event(&event);
break;
case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
mouse_handle_event(&event);
break;
case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
mouse_handle_event(&event);
break;
case ALLEGRO_EVENT_KEY_DOWN:
keyboard_event(&event);
break;
case ALLEGRO_EVENT_KEY_CHAR:
keyboard_event(&event);
break;
case ALLEGRO_EVENT_KEY_UP:
keyboard_event(&event);
break;
case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
gamepad_event(&event);
break;
case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
gamepad_event(&event);
break;
case ALLEGRO_EVENT_JOYSTICK_AXIS:
gamepad_event(&event);
break;
case ALLEGRO_EVENT_TOUCH_BEGIN:
gamepad_event(&event);
break;
case ALLEGRO_EVENT_TOUCH_END:
gamepad_event(&event);
break;
case ALLEGRO_EVENT_DISPLAY_CLOSE:
closed = true;
break;
case ALLEGRO_EVENT_TIMER:
if (!paused)
timer++;
break;
case ALLEGRO_EVENT_DISPLAY_ORIENTATION:
if (event.display.orientation == ALLEGRO_DISPLAY_ORIENTATION_90_DEGREES ||
event.display.orientation == ALLEGRO_DISPLAY_ORIENTATION_270_DEGREES)
screen_orientation = event.display.orientation;
break;
case ALLEGRO_EVENT_DISPLAY_RESIZE:
al_acknowledge_resize(screen);
screen_width = al_get_display_width(screen);
screen_height = al_get_display_height(screen);
if (fullscreen == 0) {
window_width = screen_width;
window_height = screen_height;
}
break;
case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
background_mode = true;
al_acknowledge_drawing_halt(screen);
break;
case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
background_mode = false;
break;
case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
paused = true;
break;
case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
paused = false;
break;
}
if (!al_is_event_queue_empty(event_queue)) continue;
while (timer > 0) {
--timer;
if (key_pressed(ALLEGRO_KEY_F12)) {
if (F12 == 0) {
F12 = 1;
take_screenshot(screenshot);
}
} else {
F12 = 0;
}
if (transition) {
if (update_transition(transition) == 0) {
destroy_transition(transition);
transition = NULL;
if (state[last_state].deinit) {
state[last_state].deinit();
}
if (current_state == DEMO_STATE_EXIT) {
done = true;
break;
}
}
}
else {
next_state = state[current_state].update();
if (next_state == DEMO_STATE_EXIT) {
transition =
create_transition(&state[current_state], NULL, 0.3f);
last_state = current_state;
current_state = next_state;
break;
}
else if (next_state != state[current_state].id()) {
last_state = current_state;
for (i = 0; i < state_count; i++) {
if (state[i].id() == next_state) {
state[i].init();
transition =
create_transition(&state[current_state], &state[i],
0.3f);
current_state = i;
break;
}
}
}
}
need_to_redraw = 1;
fps_tick(fps);
keyboard_tick();
mouse_tick();
}
if (need_to_redraw == 1 && !background_mode) {
draw_framework();
need_to_redraw = 0;
fps_frame(fps);
}
done = done || closed;
}
}
void shutdown_framework(void)
{
al_destroy_event_queue(event_queue);
write_global_config(config_path);
destroy_screenshot(screenshot);
destroy_fps(fps);
destroy_vcontroller(controller[DEMO_CONTROLLER_KEYBOARD], config_path);
destroy_vcontroller(controller[DEMO_CONTROLLER_GAMEPAD], config_path);
destroy_game();
unload_data();
destroy_credits();
if (transition) {
destroy_transition(transition);
}
}
