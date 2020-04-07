#include <stdio.h>
#define ALLEGRO_UNSTABLE
#include <allegro5/allegro.h>
#include "common.c"
#define WIDTH 640
#define HEIGHT 480
#define SIZE_LOG 50
ALLEGRO_EVENT_QUEUE *event_queue;
ALLEGRO_DISPLAY *display;
static void log_key(char const *how, int keycode, int unichar, int modifiers)
{
char multibyte[5] = {0, 0, 0, 0, 0};
const char* key_name;
al_utf8_encode(multibyte, unichar <= 32 ? ' ' : unichar);
key_name = al_keycode_to_name(keycode);
log_printf("%-8s code=%03d, char='%s' (%4d), modifiers=%08x, [%s]\n",
how, keycode, multibyte, unichar, modifiers, key_name);
}
static void main_loop(void)
{
ALLEGRO_EVENT event;
log_printf("Focus on the main window (black) and press keys to see events. ");
log_printf("Escape quits.\n\n");
while (true) {
al_wait_for_event(event_queue, &event);
switch (event.type) {
case ALLEGRO_EVENT_KEY_DOWN:
if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
return;
}
log_key("KEY_DOWN", event.keyboard.keycode, 0, 0);
break;
case ALLEGRO_EVENT_KEY_UP:
log_key("KEY_UP", event.keyboard.keycode, 0, 0);
break;
case ALLEGRO_EVENT_KEY_CHAR: {
char const *label = (event.keyboard.repeat ? "repeat" : "KEY_CHAR");
log_key(label,
event.keyboard.keycode,
event.keyboard.unichar,
event.keyboard.modifiers);
break;
}
case ALLEGRO_EVENT_DISPLAY_CLOSE:
return;
case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
al_clear_keyboard_state(event.display.source);
log_printf("Cleared keyboard state\n");
break;
default:
break;
}
}
}
int main(int argc, char **argv)
{
(void)argc;
(void)argv;
if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
open_log_monospace();
display = al_create_display(WIDTH, HEIGHT);
if (!display) {
abort_example("al_create_display failed\n");
}
al_clear_to_color(al_map_rgb_f(0, 0, 0));
al_flip_display();
if (!al_install_keyboard()) {
abort_example("al_install_keyboard failed\n");
}
event_queue = al_create_event_queue();
if (!event_queue) {
abort_example("al_create_event_queue failed\n");
}
al_register_event_source(event_queue, al_get_keyboard_event_source());
al_register_event_source(event_queue, al_get_display_event_source(display));
main_loop();
close_log(false);
return 0;
}
