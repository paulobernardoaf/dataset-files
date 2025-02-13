#include "allegro5/allegro.h"
typedef struct SLICE_DATA SLICE_DATA;
enum {
SLICE_READ = 1,
SLICE_WRITE = 2,
SLICE_EXPANDABLE = 4
};
struct SLICE_DATA
{
ALLEGRO_FILE *fp; 
size_t anchor; 
size_t pos; 
size_t size; 
int mode;
};
static bool slice_fclose(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
bool ret;
ret = al_fseek(slice->fp, slice->anchor + slice->size, ALLEGRO_SEEK_SET);
al_free(slice);
return ret;
}
static size_t slice_fread(ALLEGRO_FILE *f, void *ptr, size_t size)
{
SLICE_DATA *slice = al_get_file_userdata(f);
if (!(slice->mode & SLICE_READ)) {
return 0;
}
if (!(slice->mode & SLICE_EXPANDABLE) && slice->pos + size > slice->size) {
size = slice->size - slice->pos;
}
if (!size) {
return 0;
}
else {
size_t b = al_fread(slice->fp, ptr, size);
slice->pos += b;
if (slice->pos > slice->size)
slice->size = slice->pos;
return b;
}
}
static size_t slice_fwrite(ALLEGRO_FILE *f, const void *ptr, size_t size)
{
SLICE_DATA *slice = al_get_file_userdata(f);
if (!(slice->mode & SLICE_WRITE)) {
return 0;
}
if (!(slice->mode & SLICE_EXPANDABLE) && slice->pos + size > slice->size) {
size = slice->size - slice->pos;
}
if (!size) {
return 0;
}
else {
size_t b = al_fwrite(slice->fp, ptr, size);
slice->pos += b;
if (slice->pos > slice->size)
slice->size = slice->pos;
return b;
}
}
static bool slice_fflush(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return al_fflush(slice->fp);
}
static int64_t slice_ftell(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return slice->pos;
}
static bool slice_fseek(ALLEGRO_FILE *f, int64_t offset, int whence)
{
SLICE_DATA *slice = al_get_file_userdata(f);
if (whence == ALLEGRO_SEEK_SET) {
offset = slice->anchor + offset;
}
else if (whence == ALLEGRO_SEEK_CUR) {
offset = slice->anchor + slice->pos + offset;
}
else if (whence == ALLEGRO_SEEK_END) {
offset = slice->anchor + slice->size + offset;
}
else {
return false;
}
if ((size_t) offset < slice->anchor) {
offset = slice->anchor;
}
else if ((size_t) offset > slice->anchor + slice->size) {
if (!(slice->mode & SLICE_EXPANDABLE)) {
offset = slice->anchor + slice->size;
}
}
if (al_fseek(slice->fp, offset, ALLEGRO_SEEK_SET)) {
slice->pos = offset - slice->anchor;
if (slice->pos > slice->size)
slice->size = slice->pos;
return true;
}
return false;
}
static bool slice_feof(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return slice->pos >= slice->size;
}
static int slice_ferror(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return al_ferror(slice->fp);
}
static const char *slice_ferrmsg(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return al_ferrmsg(slice->fp);
}
static void slice_fclearerr(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
al_fclearerr(slice->fp);
}
static off_t slice_fsize(ALLEGRO_FILE *f)
{
SLICE_DATA *slice = al_get_file_userdata(f);
return slice->size;
}
static const ALLEGRO_FILE_INTERFACE fi =
{
NULL,
slice_fclose,
slice_fread,
slice_fwrite,
slice_fflush,
slice_ftell,
slice_fseek,
slice_feof,
slice_ferror,
slice_ferrmsg,
slice_fclearerr,
NULL,
slice_fsize
};
ALLEGRO_FILE *al_fopen_slice(ALLEGRO_FILE *fp, size_t initial_size, const char *mode)
{
SLICE_DATA *userdata = al_calloc(1, sizeof(*userdata));
if (!userdata) {
return NULL;
}
if (strstr(mode, "r") || strstr(mode, "R")) {
userdata->mode |= SLICE_READ;
}
if (strstr(mode, "w") || strstr(mode, "W")) {
userdata->mode |= SLICE_WRITE;
}
if (strstr(mode, "e") || strstr(mode, "E")) {
userdata->mode |= SLICE_EXPANDABLE;
}
userdata->fp = fp;
userdata->anchor = al_ftell(fp);
userdata->size = initial_size;
return al_create_file_handle(&fi, userdata);
}
