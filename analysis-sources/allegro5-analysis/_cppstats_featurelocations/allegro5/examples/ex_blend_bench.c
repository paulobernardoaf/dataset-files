



#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <time.h>

#include "common.c"





#define WARMUP 100




#define TEST_TIME 5.0

enum Mode {
ALL,
PLAIN_BLIT,
SCALED_BLIT,
ROTATE_BLIT
};

static char const *names[] = {
"", "Plain blit", "Scaled blit", "Rotated blit"
};

ALLEGRO_DISPLAY *display;

static void step(enum Mode mode, ALLEGRO_BITMAP *b2)
{
switch (mode) {
case ALL: break;
case PLAIN_BLIT:
al_draw_bitmap(b2, 0, 0, 0);
break;
case SCALED_BLIT:
al_draw_scaled_bitmap(b2, 0, 0, 320, 200, 0, 0, 640, 480, 0);
break;
case ROTATE_BLIT:
al_draw_scaled_rotated_bitmap(b2, 10, 10, 10, 10, 2.0, 2.0,
ALLEGRO_PI/30, 0);
break;
}
}




static double current_clock(void)
{
clock_t c = clock();
return (double)c / CLOCKS_PER_SEC;
}

static bool do_test(enum Mode mode)
{
ALLEGRO_STATE state;
ALLEGRO_BITMAP *b1;
ALLEGRO_BITMAP *b2;
int REPEAT;
double t0, t1;
int i;

al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

b1 = al_load_bitmap("data/mysha.pcx");
if (!b1) {
abort_example("Error loading data/mysha.pcx\n");
return false;
}

b2 = al_load_bitmap("data/allegro.pcx");
if (!b2) {
abort_example("Error loading data/mysha.pcx\n");
return false;
}

al_set_target_bitmap(b1);
al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
step(mode, b2);


al_store_state(&state, ALLEGRO_STATE_ALL);
al_set_target_backbuffer(display);
al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
al_draw_bitmap(b1, 0, 0, 0);
al_flip_display();
al_restore_state(&state);

log_printf("Benchmark: %s\n", names[mode]);
log_printf("Please wait...\n");


t0 = current_clock();
for (i = 0; i < WARMUP; i++) {
step(mode, b2);
}
t1 = current_clock();
REPEAT = TEST_TIME * 100 / (t1 - t0);


t0 = current_clock();
for (i = 0; i < REPEAT; i++) {
step(mode, b2);
}
t1 = current_clock();

log_printf("Time = %g s, %d steps\n",
t1 - t0, REPEAT);
log_printf("%s: %g FPS\n", names[mode], REPEAT / (t1 - t0));
log_printf("Done\n");

al_destroy_bitmap(b1);
al_destroy_bitmap(b2);

return true;
}

int main(int argc, char **argv)
{
enum Mode mode = ALL;
int i;

if (argc > 1) {
i = strtol(argv[1], NULL, 10);
switch (i) {
case 0:
mode = PLAIN_BLIT;
break;
case 1:
mode = SCALED_BLIT;
break;
case 2:
mode = ROTATE_BLIT;
break;
}
}

if (!al_init()) {
abort_example("Could not init Allegro\n");
}

open_log();

al_init_image_addon();
al_init_primitives_addon();
init_platform_specific();

display = al_create_display(640, 480);
if (!display) {
abort_example("Error creating display\n");
}

if (mode == ALL) {
for (mode = PLAIN_BLIT; mode <= ROTATE_BLIT; mode++) {
do_test(mode);
}
}
else {
do_test(mode);
}

al_destroy_display(display);

close_log(true);

return 0;
}


