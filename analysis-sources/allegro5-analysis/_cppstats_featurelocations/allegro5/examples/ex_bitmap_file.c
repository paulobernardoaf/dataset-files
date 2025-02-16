




#include <stdio.h>
#include <stdlib.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"

#include "common.c"

int main(int argc, char **argv)
{
const char *filename;
char *fileextension = NULL;
ALLEGRO_DISPLAY *display;
ALLEGRO_FILE *file;
ALLEGRO_BITMAP *bitmap;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT_QUEUE *queue;

bool redraw = true;
double zoom = 1;
double t0;
double t1;








if (argc > 1) {
filename = argv[1];
}
else {
filename = "data/mysha.pcx";
}

if (!al_init()) {
abort_example("Could not init Allegro.\n");
}


open_log();




if (argc > 2) {
al_set_new_display_adapter(atoi(argv[2]));
}




al_install_mouse();
al_install_keyboard();




al_init_image_addon();


init_platform_specific();


display = al_create_display(640, 480);
if (!display) {
abort_example("Error creating display\n");
}

al_set_window_title(display, filename);


t0 = al_get_time();
file = al_fopen(filename, "rb");
if(file) {
fileextension = strrchr(filename, '.');
bitmap = al_load_bitmap_f(file, fileextension);
al_fclose( file );
}
t1 = al_get_time();
if (!bitmap) {
abort_example("%s not found or failed to load\n", filename);
}

log_printf("Loading took %.4f seconds\n", t1 - t0);


timer = al_create_timer(1.0 / 30);
queue = al_create_event_queue();
al_register_event_source(queue, al_get_keyboard_event_source());
al_register_event_source(queue, al_get_display_event_source(display));
al_register_event_source(queue, al_get_timer_event_source(timer));
al_start_timer(timer); 


while (1) {
ALLEGRO_EVENT event;
al_wait_for_event(queue, &event); 
if (event.type == ALLEGRO_EVENT_DISPLAY_ORIENTATION) {
int o = event.display.orientation;
if (o == ALLEGRO_DISPLAY_ORIENTATION_0_DEGREES) {
log_printf("0 degrees\n");
}
else if (o == ALLEGRO_DISPLAY_ORIENTATION_90_DEGREES) {
log_printf("90 degrees\n");
}
else if (o == ALLEGRO_DISPLAY_ORIENTATION_180_DEGREES) {
log_printf("180 degrees\n");
}
else if (o == ALLEGRO_DISPLAY_ORIENTATION_270_DEGREES) {
log_printf("270 degrees\n");
}
else if (o == ALLEGRO_DISPLAY_ORIENTATION_FACE_UP) {
log_printf("Face up\n");
}
else if (o == ALLEGRO_DISPLAY_ORIENTATION_FACE_DOWN) {
log_printf("Face down\n");
}
}
if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
break;






if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
break; 
if (event.keyboard.unichar == '1')
zoom = 1;
if (event.keyboard.unichar == '+')
zoom *= 1.1;
if (event.keyboard.unichar == '-')
zoom /= 1.1;
if (event.keyboard.unichar == 'f')
zoom = (double)al_get_display_width(display) /
al_get_bitmap_width(bitmap);
}


if (event.type == ALLEGRO_EVENT_TIMER)
redraw = true;


if (redraw && al_is_event_queue_empty(queue)) {
redraw = false;

al_clear_to_color(al_map_rgb_f(0, 0, 0));
if (zoom == 1)
al_draw_bitmap(bitmap, 0, 0, 0);
else
al_draw_scaled_rotated_bitmap(
bitmap, 0, 0, 0, 0, zoom, zoom, 0, 0);
al_flip_display();
}
}

al_destroy_bitmap(bitmap);

close_log(false);

return 0;
}


