





#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"

#include "common.c"

static const int MAX_REGION = 4;

static float tints[12] =
{
4.0, 0.0, 1.0,
0.0, 4.0, 1.0,
1.0, 0.0, 4.0,
4.0, 4.0, 1.0
};

static void choose_shader_source(ALLEGRO_SHADER_PLATFORM platform,
char const **vsource, char const **psource)
{
if (platform == ALLEGRO_SHADER_HLSL) {
*vsource = "data/ex_shader_vertex.hlsl";
*psource = "data/ex_shader_pixel.hlsl";
}
else if (platform == ALLEGRO_SHADER_GLSL) {
*vsource = "data/ex_shader_vertex.glsl";
*psource = "data/ex_shader_pixel.glsl";
}
else {

*vsource = NULL;
*psource = NULL;
}
}

static ALLEGRO_BITMAP *make_region(ALLEGRO_BITMAP *parent,
int x, int y, int w, int h, ALLEGRO_SHADER *shader)
{
ALLEGRO_BITMAP *sub = al_create_sub_bitmap(parent, x, y, w, h);
if (sub) {
al_set_target_bitmap(sub);
al_use_shader(shader);

}
return sub;
}

int main(int argc, char **argv)
{
ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP *image;
ALLEGRO_BITMAP *backbuffer;
ALLEGRO_BITMAP *region[4];
ALLEGRO_SHADER *shader;
const char *vsource;
const char *psource;
ALLEGRO_TRANSFORM t;
int i;

(void)argc;
(void)argv;

if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
al_install_keyboard();
al_init_image_addon();
init_platform_specific();

al_set_new_display_flags(ALLEGRO_PROGRAMMABLE_PIPELINE);
display = al_create_display(640, 480);
if (!display) {
abort_example("Could not create display.\n");
}

image = al_load_bitmap("data/mysha.pcx");
if (!image) {
abort_example("Could not load image.\n");
}


shader = al_create_shader(ALLEGRO_SHADER_AUTO);
if (!shader) {
abort_example("Could not create shader.\n");
}
choose_shader_source(al_get_shader_platform(shader), &vsource, &psource);
if (!vsource|| !psource) {
abort_example("Could not load source files.\n");
}
if (!al_attach_shader_source_file(shader, ALLEGRO_VERTEX_SHADER, vsource)) {
abort_example("al_attach_shader_source_file failed: %s\n",
al_get_shader_log(shader));
}
if (!al_attach_shader_source_file(shader, ALLEGRO_PIXEL_SHADER, psource)) {
abort_example("al_attach_shader_source_file failed: %s\n",
al_get_shader_log(shader));
}
if (!al_build_shader(shader)) {
abort_example("al_build_shader failed: %s\n", al_get_shader_log(shader));
}


backbuffer = al_get_backbuffer(display);
region[0] = make_region(backbuffer, 0, 0, 320, 200, shader);
region[1] = make_region(backbuffer, 320, 0, 320, 200, shader);
region[2] = make_region(backbuffer, 0, 240, 320, 200, shader);
region[3] = make_region(backbuffer, 320, 240, 320, 200, shader);
if (!region[0] || !region[1] || !region[2] || !region[3]) {
abort_example("make_region failed\n");
}


al_identity_transform(&t);
al_scale_transform(&t, 2.0, 2.0);
al_translate_transform(&t, -160, -100);
al_use_transform(&t);

for (;;) {
ALLEGRO_KEYBOARD_STATE s;
al_get_keyboard_state(&s);
if (al_key_down(&s, ALLEGRO_KEY_ESCAPE))
break;

for (i = 0; i < MAX_REGION; i++) {





al_set_target_bitmap(region[i]);
al_set_shader_float_vector("tint", 3, &tints[i * 3], 1);
al_draw_bitmap(image, 0, 0, 0);
}

al_set_target_backbuffer(display);
al_draw_tinted_bitmap(image, al_map_rgba_f(0.5, 0.5, 0.5, 0.5),
320/2, 240/2, 0);

al_flip_display();
}

al_set_target_backbuffer(display);
al_use_shader(NULL);
al_destroy_shader(shader);

return 0;
}


