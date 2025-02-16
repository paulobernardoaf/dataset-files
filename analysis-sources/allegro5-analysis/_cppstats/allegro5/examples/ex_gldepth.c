#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_opengl.h"
#include "allegro5/allegro_font.h"
#include "common.c"
struct camera {
double xangle, yangle, zangle;
double dist;
};
struct camera camera = {
0.0, 0.0, 0.0,
20.0
};
double angle_speed = 5.0;
double dist_speed = 1.0;
GLuint tex;
ALLEGRO_BITMAP *bmp;
bool key[ALLEGRO_KEY_MAX];
static void set_camera_position(void)
{
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 40.0);
glTranslatef(0, 0, -camera.dist);
glRotatef(camera.xangle, 1, 0, 0);
glRotatef(camera.yangle, 0, 1, 0);
glRotatef(camera.zangle, 0, 0, 1);
glMatrixMode(GL_MODELVIEW);
}
static void keyboard(void)
{
if(key[ALLEGRO_KEY_LEFT]) camera.yangle += angle_speed;
if(key[ALLEGRO_KEY_RIGHT]) camera.yangle -= angle_speed;
if(key[ALLEGRO_KEY_UP]) camera.xangle += angle_speed;
if(key[ALLEGRO_KEY_DOWN]) camera.xangle -= angle_speed;
if(key[ALLEGRO_KEY_PGUP]) camera.dist -= dist_speed;
if(key[ALLEGRO_KEY_PGDN]) camera.dist += dist_speed;
}
static void draw(void)
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glLoadIdentity();
glTranslatef(-2.5, 0.0, 0.0);
glRotatef(-30, 1.0, 0.0, 0.0);
glRotatef(30, 0.0, 1.0, 0.0);
glRotatef(30, 0.0, 0.0, 1.0);
glColor3f(1.0, 0.0, 1.0);
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, tex);
glBegin(GL_TRIANGLE_FAN);
glTexCoord2f(0, 0); glVertex3d(0, 4, 0);
glTexCoord2f(1, 0); glVertex3d(0, -4, -4);
glTexCoord2f(1, 1); glVertex3d(-4, -4, 4);
glTexCoord2f(0, 1); glVertex3d(4, -4, 4);
glTexCoord2f(1, 0); glVertex3d(0, -4, -4);
glEnd();
glColor3f(0.0, 1.0, 1.0);
glBegin(GL_TRIANGLES);
glTexCoord2f(1, 0); glVertex3d(0, -4, -4);
glTexCoord2f(0, 1); glVertex3d(4, -4, 4);
glTexCoord2f(1, 1); glVertex3d(-4, -4, 4);
glEnd();
glLoadIdentity();
glTranslatef(2.5, 0.0, 0.0);
glRotatef(45, 1.0, 0.0, 0.0);
glRotatef(45, 0.0, 1.0, 0.0);
glRotatef(45, 0.0, 0.0, 1.0);
glColor3f(0.0, 1.0, 0.0);
glDisable(GL_TEXTURE_2D);
glBegin(GL_QUAD_STRIP);
glVertex3d(3, 3, -3);
glVertex3d(3, -3, -3);
glVertex3d(-3, 3, -3);
glVertex3d(-3, -3, -3);
glVertex3d(-3, 3, 3);
glVertex3d(-3, -3, 3);
glVertex3d(3, 3, 3);
glVertex3d(3, -3, 3);
glVertex3d(3, 3, -3);
glVertex3d(3, -3, -3);
glEnd();
glColor3f(0.0, 0.0, 1.0);
glBegin(GL_QUADS);
glVertex3d(-3, -3, -3);
glVertex3d(3, -3, -3);
glVertex3d(3, -3, 3);
glVertex3d(-3, -3, 3);
glEnd();
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, tex);
glBegin(GL_QUADS);
glTexCoord2f(0, 0); glVertex3d(-3, 3, -3);
glTexCoord2f(1, 0); glVertex3d(-3, 3, 3);
glTexCoord2f(1, 1); glVertex3d(3, 3, 3);
glTexCoord2f(0, 1); glVertex3d(3, 3, -3);
glEnd();
}
static void setup_textures(ALLEGRO_DISPLAY *display)
{
ALLEGRO_BITMAP *tmp_bmp;
ALLEGRO_FONT *font;
int w, h, depth;
font = al_load_font("data/fixed_font.tga", 0, 0);
if(!font) {
abort_example("Error loading `data/fixed_font.tga'\n");
}
tmp_bmp = al_load_bitmap("data/mysha.pcx");
if(!tmp_bmp) {
abort_example("Error loading `data/mysha.pcx'\n");
}
w = 128;
h = 128;
bmp = al_create_bitmap(w, h);
al_set_target_bitmap(bmp);
al_draw_scaled_bitmap(tmp_bmp,
0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
0, 0, w, h, 0);
depth = al_get_display_option(display, ALLEGRO_DEPTH_SIZE);
if (!depth)
al_draw_textf(font, al_map_rgb(255, 0, 0), 0, 5, 0, "No Z-buffer!");
else
al_draw_textf(font, al_map_rgb(255, 0, 0), 0, 5, 0, "Z-buffer: %i bits", depth);
al_set_target_backbuffer(display);
al_destroy_bitmap(tmp_bmp);
al_destroy_font(font);
glEnable(GL_TEXTURE_2D);
glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
tex = al_get_opengl_texture(bmp);
}
int main(void)
{
ALLEGRO_DISPLAY *display;
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT event;
if(!al_init()) {
abort_example("Could not init Allegro.\n");
}
al_init_image_addon();
al_init_font_addon();
al_install_keyboard();
al_set_new_display_flags(ALLEGRO_OPENGL);
al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
display = al_create_display(640, 480);
if(!display) {
abort_example("Could not create display.\n");
}
timer = al_create_timer(1. / 60.);
queue = al_create_event_queue();
al_register_event_source(queue,al_get_keyboard_event_source());
al_register_event_source(queue,al_get_display_event_source(display));
al_register_event_source(queue,al_get_timer_event_source(timer));
glEnable(GL_DEPTH_TEST);
glDisable(GL_CULL_FACE);
setup_textures(display);
al_start_timer(timer);
while(true) {
al_wait_for_event(queue, &event);
switch(event.type) {
case ALLEGRO_EVENT_DISPLAY_CLOSE:
goto done;
case ALLEGRO_EVENT_KEY_DOWN:
if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
goto done;
key[event.keyboard.keycode] = true;
break;
case ALLEGRO_EVENT_KEY_UP:
key[event.keyboard.keycode] = false;
break;
case ALLEGRO_EVENT_TIMER:
keyboard();
if(al_is_event_queue_empty(queue)) {
set_camera_position();
draw();
al_flip_display();
}
break;
}
}
done:
al_destroy_bitmap(bmp);
return 0;
}
