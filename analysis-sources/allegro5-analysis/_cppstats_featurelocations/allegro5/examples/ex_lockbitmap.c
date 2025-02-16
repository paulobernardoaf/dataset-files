





#define ALLEGRO_UNSTABLE
#include "allegro5/allegro.h"

#include "common.c"

enum Mode
{
MODE_VIDEO,
MODE_MEMORY,
MODE_BACKBUFFER
};

static void fill(ALLEGRO_BITMAP *bitmap, int lock_flags)
{
ALLEGRO_LOCKED_REGION *locked;
uint8_t *ptr;
int i, j;








locked = al_lock_bitmap_region(bitmap, 193, 65, 3*127, 127,
ALLEGRO_PIXEL_FORMAT_RGB_565, lock_flags);
if (!locked)
return;

for (j = 0; j < 127; j++) {
ptr = (uint8_t *)locked->data + j * locked->pitch;

for (i = 0; i < 3*127; i++) {
uint8_t red;
uint8_t green;
uint8_t blue;
uint16_t col;
uint16_t *cptr = (uint16_t *)ptr;

if (j == 0 || j == 126 || i == 0 || i == 3*127-1) {
red = green = blue = 0;
}
else if (i < 127) {
red = 255;
green = blue = j*2;
}
else if (i < 2*127) {
green = 255;
red = blue = j*2;
}
else {
blue = 255;
red = green = j*2;
}










if ((lock_flags & ALLEGRO_LOCK_WRITEONLY) || (j + i) % 2 == 1) {
col = ((red/8) << 11) | ((green/4) << 5) | (blue/8);
*cptr = col;
}
ptr += 2;
}
}
al_unlock_bitmap(bitmap);
}

static void draw(ALLEGRO_DISPLAY *display, enum Mode mode, int lock_flags)
{
ALLEGRO_BITMAP *bitmap;


if (mode == MODE_VIDEO) {
log_printf("Locking video bitmap");
al_clear_to_color(al_map_rgb(0, 0, 0));
al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
bitmap = al_create_bitmap(3*256, 256);
}
else if (mode == MODE_MEMORY) {
log_printf("Locking memory bitmap");
al_clear_to_color(al_map_rgb(0, 0, 0));
al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
bitmap = al_create_bitmap(3*256, 256);
}
else {
log_printf("Locking display backbuffer");
bitmap = al_get_backbuffer(display);
}
if (!bitmap) {
abort_example("Error creating bitmap");
}

if (lock_flags & ALLEGRO_LOCK_WRITEONLY) {
log_printf(" in write-only mode\n");
}
else {
log_printf(" in read/write mode\n");
}

al_set_target_bitmap(bitmap);
al_clear_to_color(al_map_rgb_f(0.8, 0.8, 0.9));
al_set_target_backbuffer(display);

fill(bitmap, lock_flags);

if (mode != MODE_BACKBUFFER) {
al_draw_bitmap(bitmap, 0, 0, 0);
al_destroy_bitmap(bitmap);
bitmap = NULL;
}

al_flip_display();
}

static enum Mode cycle_mode(enum Mode mode)
{
switch (mode) {
case MODE_VIDEO:
return MODE_MEMORY;
case MODE_MEMORY:
return MODE_BACKBUFFER;
case MODE_BACKBUFFER:
default:
return MODE_VIDEO;
}
}

static int toggle_writeonly(int lock_flags)
{
return lock_flags ^ ALLEGRO_LOCK_WRITEONLY;
}

int main(int argc, char **argv)
{
ALLEGRO_DISPLAY *display;
ALLEGRO_EVENT_QUEUE *events;
ALLEGRO_EVENT event;
enum Mode mode = MODE_VIDEO;
int lock_flags = ALLEGRO_LOCK_WRITEONLY;
bool redraw = true;

(void)argc;
(void)argv;

if (!al_init()) {
abort_example("Could not init Allegro.\n");
}

al_install_keyboard();
al_install_mouse();
al_install_touch_input();

open_log();


display = al_create_display(3*256, 256);
if (!display) {
abort_example("Error creating display\n");
}

events = al_create_event_queue();
al_register_event_source(events, al_get_keyboard_event_source());
al_register_event_source(events, al_get_mouse_event_source());
if (al_is_touch_input_installed()) {
al_register_event_source(events,
al_get_touch_input_mouse_emulation_event_source());
}

log_printf("Press space to change bitmap type\n");
log_printf("Press w to toggle WRITEONLY mode\n");

for (;;) {
if (redraw) {
draw(display, mode, lock_flags);
redraw = false;
}

al_wait_for_event(events, &event);
if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
break;
if (event.keyboard.unichar == ' ') {
mode = cycle_mode(mode);
redraw = true;
}
else if (event.keyboard.unichar == 'w' || event.keyboard.unichar == 'W') {
lock_flags = toggle_writeonly(lock_flags);
redraw = true;
}
}
else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
if (event.mouse.button == 1) {
if (event.mouse.x < al_get_display_width(display) / 2) {
mode = cycle_mode(mode);
}
else {
lock_flags = toggle_writeonly(lock_flags);
}
redraw = true;
}
}
}

close_log(false);
return 0;
}


