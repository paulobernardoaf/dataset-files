#include <stdio.h>
#include <string.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include "common.c"
typedef struct CURL_FILE CURL_FILE;
struct CURL_FILE {
CURL *curl;
char *buffer; 
size_t buffer_len; 
size_t buffer_pos; 
int still_running; 
};
static ALLEGRO_FILE_INTERFACE curl_file_vtable;
static CURLM *multi_handle;
static size_t write_callback(char *buffer, size_t size, size_t nitems,
void *userp)
{
CURL_FILE *cf = userp;
char *newbuff;
size_t rembuff;
size *= nitems;
rembuff = cf->buffer_len - cf->buffer_pos;
if (size > rembuff) {
newbuff = realloc(cf->buffer, cf->buffer_len + size - rembuff);
if (!newbuff) {
log_printf("callback buffer grow failed\n");
size = rembuff;
}
else {
cf->buffer_len += size - rembuff;
cf->buffer = newbuff;
}
}
memcpy(cf->buffer + cf->buffer_pos, buffer, size);
cf->buffer_pos += size;
return size;
}
static void *curl_file_fopen(const char *path, const char *mode)
{
CURL_FILE *cf;
if (strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0)
return NULL;
cf = calloc(1, sizeof(*cf));
if (!cf)
return NULL;
cf->curl = curl_easy_init();
curl_easy_setopt(cf->curl, CURLOPT_URL, path);
curl_easy_setopt(cf->curl, CURLOPT_WRITEDATA, cf);
curl_easy_setopt(cf->curl, CURLOPT_VERBOSE, 1);
curl_easy_setopt(cf->curl, CURLOPT_WRITEFUNCTION, write_callback);
if (!multi_handle)
multi_handle = curl_multi_init();
curl_multi_add_handle(multi_handle, cf->curl);
while (curl_multi_perform(multi_handle, &cf->still_running) ==
CURLM_CALL_MULTI_PERFORM);
if ((cf->buffer_pos == 0) && (!cf->still_running)) {
curl_multi_remove_handle(multi_handle, cf->curl);
curl_easy_cleanup(cf->curl);
free(cf);
cf = NULL;
}
return cf;
}
static bool curl_file_fclose(ALLEGRO_FILE *f)
{
CURL_FILE *cf = al_get_file_userdata(f);
curl_multi_remove_handle(multi_handle, cf->curl);
curl_easy_cleanup(cf->curl);
if (cf->buffer)
free(cf->buffer);
free(cf);
return true;
}
static bool fill_buffer(CURL_FILE *cf, size_t size)
{
fd_set fdread;
fd_set fdwrite;
fd_set fdexcep;
struct timeval timeout;
int rc;
if (!cf->still_running || cf->buffer_pos > size)
return false;
do {
int maxfd = -1;
CURLMcode mc;
FD_ZERO(&fdread);
FD_ZERO(&fdwrite);
FD_ZERO(&fdexcep);
timeout.tv_sec = 10;
timeout.tv_usec = 0;
mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
if(mc != CURLM_OK) {
abort_example("curl_multi_fdset() failed, code %d.\n", mc);
}
if (maxfd > -1) {
rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
}
else {
al_rest(0.1);
rc = 0;
}
switch (rc) {
case -1:
break;
case 0:
default:
curl_multi_perform(multi_handle, &cf->still_running);
break;
}
} while (cf->still_running && cf->buffer_pos < size);
return true;
}
static void use_buffer(CURL_FILE *cf, size_t size)
{
if (cf->buffer_pos - size <= 0) {
cf->buffer_pos = 0;
}
else {
memmove(cf->buffer, cf->buffer + size, cf->buffer_pos - size);
cf->buffer_pos -= size;
}
}
static size_t curl_file_fread(ALLEGRO_FILE *f, void *ptr, size_t size)
{
CURL_FILE *cf = al_get_file_userdata(f);
fill_buffer(cf, size);
if (!cf->buffer_pos)
return 0;
if (cf->buffer_pos < size)
size = cf->buffer_pos;
memcpy(ptr, cf->buffer, size);
use_buffer(cf, size);
return size;
}
static size_t curl_file_fwrite(ALLEGRO_FILE *f, const void *ptr, size_t size)
{
(void)f;
(void)ptr;
(void)size;
al_set_errno(EBADF);
return 0;
}
static bool curl_file_fflush(ALLEGRO_FILE *f)
{
(void)f;
return true;
}
static int64_t curl_file_ftell(ALLEGRO_FILE *f)
{
(void)f;
al_set_errno(ENOSYS);
return -1;
}
static bool curl_file_fseek(ALLEGRO_FILE *f, int64_t offset, int whence)
{
if (whence != ALLEGRO_SEEK_CUR || offset < 0) {
al_set_errno(ENOSYS);
return false;
}
while (offset > 0) {
if (al_fgetc(f) == EOF)
break;
offset--;
}
return offset == 0;
}
static bool curl_file_feof(ALLEGRO_FILE *f)
{
CURL_FILE *cf = al_get_file_userdata(f);
return (cf->buffer_pos == 0 && !cf->still_running);
}
static int curl_file_ferror(ALLEGRO_FILE *f)
{
(void)f;
return false;
}
static const char *curl_file_ferrmsg(ALLEGRO_FILE *f)
{
(void)f;
return "";
}
static void curl_file_fclearerr(ALLEGRO_FILE *f)
{
(void)f;
}
static int curl_file_fungetc(ALLEGRO_FILE *f, int c)
{
(void)f;
(void)c;
al_set_errno(ENOSYS);
return -1;
}
static off_t curl_file_fsize(ALLEGRO_FILE *f)
{
(void)f;
al_set_errno(ENOSYS);
return -1;
}
static ALLEGRO_FILE_INTERFACE curl_file_vtable =
{
curl_file_fopen,
curl_file_fclose,
curl_file_fread,
curl_file_fwrite,
curl_file_fflush,
curl_file_ftell,
curl_file_fseek,
curl_file_feof,
curl_file_ferror,
curl_file_ferrmsg,
curl_file_fclearerr,
curl_file_fungetc,
curl_file_fsize
};
static void show_image(ALLEGRO_BITMAP *bmp, ALLEGRO_DISPLAY *disp)
{
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_EVENT event;
queue = al_create_event_queue();
al_register_event_source(queue, al_get_keyboard_event_source());
al_register_event_source(queue, al_get_display_event_source(disp));
while (true) {
al_draw_bitmap(bmp, 0, 0, 0);
al_flip_display();
al_wait_for_event(queue, &event);
if (event.type == ALLEGRO_EVENT_KEY_DOWN
&& event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
break;
}
else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
break;
}
}
al_destroy_event_queue(queue);
}
int main(int argc, const char *argv[])
{
const char *url;
ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP *bmp;
bool wait_for_log = true;
if (argc > 1)
url = argv[1];
else
url = "http://liballeg.org/images/logo.png";
if (!al_init()) {
abort_example("Could not init Allegro.\n");
}
open_log();
al_init_image_addon();
al_install_keyboard();
display = al_create_display(640, 480);
if (!display) {
abort_example("Unable to create display.\n");
}
curl_global_init(CURL_GLOBAL_ALL);
al_set_new_file_interface(&curl_file_vtable);
bmp = al_load_bitmap(url);
if (bmp) {
show_image(bmp, display);
al_destroy_bitmap(bmp);
wait_for_log = false;
}
curl_global_cleanup();
al_destroy_display(display);
close_log(wait_for_log);
return 0;
}
