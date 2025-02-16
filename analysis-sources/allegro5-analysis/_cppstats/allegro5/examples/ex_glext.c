#define ALLEGRO_UNSTABLE
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include "common.c"
#if defined(ALLEGRO_MACOSX)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#define WINDOW_W 640
#define WINDOW_H 480
#define MESH_SIZE 64
GLfloat mesh[MESH_SIZE][MESH_SIZE][3];
GLfloat wave_movement = 0.0f;
#define SIN(d, s, t) "MAD "d", "s", one_over_pi, 0.5;\n" "FRC "d","d";\n" "MAD "d","d", two_pi, -pi;\n" "MUL "d","d", 0.98;\n" "MUL "t".x, "d", "d";\n" "MUL "t".y, "t".x, "d";\n" "MUL "t".z, "t".y, "t".x;\n" "MUL "t".w, "t".z, "t".x;\n" "MAD "d", "t".y,-inv_3_fact, "d";\n" "MAD "d", "t".z, inv_5_fact, "d";\n" "MAD "d", "t".w,-inv_7_fact, "d";\n" 
const char *program =
"!!ARBvp1.0\n"
"ATTRIB pos = vertex.position;\n"
"ATTRIB wave = vertex.attrib[1];\n"
"PARAM modelview[4] = { state.matrix.mvp };\n"
"PARAM one_over_pi = 0.1591549;\n"
"PARAM pi = 3.1415926;\n"
"PARAM two_pi = 6.2831853;\n"
"PARAM inv_3_fact = 0.1666666;\n"
"PARAM inv_5_fact = 0.00833333333;\n"
"PARAM inv_7_fact = 0.00019841269841269;\n"
"TEMP temp, temp2;\n"
"MAD temp.y, pos.x, 0.2, wave.x;\n"
SIN("temp.y", "temp.y", "temp2")
"MAD temp.x, pos.z, 0.25, wave.x;\n"
SIN("temp.x", "temp.x", "temp2")
"ADD temp.y, temp.x, temp.y;\n"
"MOV temp2, pos;\n"
"MUL temp2.y, temp.y, 2.5;\n"
"DP4 result.position.w, temp2, modelview[3];\n"
"DP4 result.position.x, temp2, modelview[0];\n"
"DP4 result.position.y, temp2, modelview[1];\n"
"DP4 result.position.z, temp2, modelview[2];\n"
"MOV result.color, vertex.color;\n"
"END";
const char *program_nv = 
"!!ARBvp1.0"
"OPTION NV_vertex_program2;"
"ATTRIB wave = vertex.attrib[1];"
"PARAM modelview[4] = { state.matrix.mvp };"
"TEMP temp;"
"TEMP pos;"
"MOV pos, vertex.position;"
"MAD temp.xz, pos, {0.2, 1.0, 0.25, 1.0}, wave.x;"
"SIN temp.x, temp.x;"
"SIN temp.z, temp.z;"
"ADD temp.y, temp.x, temp.z;"
"MUL pos.y, temp.y, 2.5;"
"DP4 result.position.w, pos, modelview[3];"
"DP4 result.position.x, pos, modelview[0];"
"DP4 result.position.y, pos, modelview[1];"
"DP4 result.position.z, pos, modelview[2];"
"MOV result.color, vertex.color;"
"END";
static void create_mesh(void)
{
int x, z;
for (x = 0; x < MESH_SIZE; x++) {
for (z = 0; z < MESH_SIZE; z++) {
mesh[x][z][0] = (float) (MESH_SIZE / 2) - x;
mesh[x][z][1] = 0.0f;
mesh[x][z][2] = (float) (MESH_SIZE / 2) - z;
}
}
}
static void draw_mesh(void)
{
int x, z;
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
for (x = 0; x < MESH_SIZE - 1; x++) {
glBegin(GL_TRIANGLE_STRIP);
for (z = 0; z < MESH_SIZE - 1; z++) {
glVertexAttrib1fARB(1, wave_movement);
glVertex3fv(&mesh[x][z][0]);
glVertex3fv(&mesh[x+1][z][0]);
wave_movement += 0.00001f;
if (wave_movement > 2 * ALLEGRO_PI) {
wave_movement = 0.0f;
}
}
glEnd();
}
glFlush();
}
int main(int argc, const char *argv[])
{
GLuint pid;
ALLEGRO_DISPLAY *d;
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_EVENT event;
ALLEGRO_TIMER *timer;
int frames = 0;
double start;
bool limited = true;
if (argc > 1 && 0 == strcmp(argv[1], "-nolimit")) {
limited = false;
}
if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
open_log();
al_set_new_display_flags(ALLEGRO_OPENGL);
al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_SUGGEST);
d = al_create_display(WINDOW_W, WINDOW_H);
if (!d) {
abort_example("Unable to open a OpenGL display.\n");
}
if (al_get_display_option(d, ALLEGRO_SAMPLE_BUFFERS)) {
log_printf("With multisampling, level %i\n", al_get_display_option(d, ALLEGRO_SAMPLES));
}
else {
log_printf("Without multisampling.\n");
}
al_install_keyboard();
queue = al_create_event_queue();
al_register_event_source(queue, al_get_keyboard_event_source());
al_register_event_source(queue, al_get_display_event_source(d));
if (limited) {
timer = al_create_timer(1/60.0);
al_register_event_source(queue, al_get_timer_event_source(timer));
al_start_timer(timer);
}
else {
timer = NULL;
}
if (al_get_opengl_extension_list()->ALLEGRO_GL_ARB_multisample) {
glEnable(GL_MULTISAMPLE_ARB);
}
if (!al_get_opengl_extension_list()->ALLEGRO_GL_ARB_vertex_program) {
abort_example("This example requires a video card that supports "
" the ARB_vertex_program extension.\n");
}
glEnable(GL_DEPTH_TEST);
glShadeModel(GL_SMOOTH);
glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
glDisable(GL_CULL_FACE);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective(45.0, WINDOW_W/(float)WINDOW_H, 0.1, 100.0);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
gluLookAt(0.0f, 20.0f, -45.0f, 0.0f, 0.0f, 0.0f, 0, 1, 0);
create_mesh();
glEnable(GL_VERTEX_PROGRAM_ARB);
glGenProgramsARB(1, &pid);
glBindProgramARB(GL_VERTEX_PROGRAM_ARB, pid);
glGetError();
if (al_get_opengl_extension_list()->ALLEGRO_GL_NV_vertex_program2_option) {
glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
strlen(program_nv), program_nv);
}
else {
glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
strlen(program), program);
}
if (glGetError()) {
const char *pgm = al_get_opengl_extension_list()->ALLEGRO_GL_NV_vertex_program2_option
? program_nv : program;
GLint error_pos;
const GLubyte *error_str = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
abort_example("Error compiling the vertex program:\n%s\n\nat "
"character: %i\n%s\n", error_str, (int)error_pos,
pgm + error_pos);
}
start = al_get_time();
while (1) {
if (limited) {
al_wait_for_event(queue, NULL);
}
if (!al_is_event_queue_empty(queue)) {
while (al_get_next_event(queue, &event)) {
switch (event.type) {
case ALLEGRO_EVENT_DISPLAY_CLOSE:
goto done;
case ALLEGRO_EVENT_KEY_DOWN:
if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
goto done;
break;
}
}
}
draw_mesh();
al_flip_display();
frames++;
}
done:
log_printf("%.1f FPS\n", frames / (al_get_time() - start));
glDeleteProgramsARB(1, &pid);
al_destroy_event_queue(queue);
al_destroy_display(d);
close_log(true);
return 0;
}
