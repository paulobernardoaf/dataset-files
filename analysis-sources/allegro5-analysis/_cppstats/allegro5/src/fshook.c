#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_fshook.h"
ALLEGRO_FS_ENTRY *al_create_fs_entry(const char *path)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(vt->fs_create_entry);
return vt->fs_create_entry(path);
}
void al_destroy_fs_entry(ALLEGRO_FS_ENTRY *fh)
{
if (fh) {
fh->vtable->fs_destroy_entry(fh);
}
}
const char *al_get_fs_entry_name(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_name(e);
}
bool al_update_fs_entry(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_update_entry(e);
}
uint32_t al_get_fs_entry_mode(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_mode(e);
}
time_t al_get_fs_entry_atime(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_atime(e);
}
time_t al_get_fs_entry_mtime(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_mtime(e);
}
time_t al_get_fs_entry_ctime(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_ctime(e);
}
off_t al_get_fs_entry_size(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_size(e);
}
bool al_remove_fs_entry(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_remove_entry(e);
}
bool al_fs_entry_exists(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_entry_exists(e);
}
bool al_open_directory(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_open_directory(e);
}
bool al_close_directory(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_close_directory(e);
}
ALLEGRO_FS_ENTRY *al_read_directory(ALLEGRO_FS_ENTRY *e)
{
ASSERT(e != NULL);
return e->vtable->fs_read_directory(e);
}
char *al_get_current_directory(void)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(vt->fs_get_current_directory);
return vt->fs_get_current_directory();
}
bool al_change_directory(const char *path)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(vt->fs_change_directory);
ASSERT(path);
return vt->fs_change_directory(path);
}
bool al_make_directory(const char *path)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(path);
ASSERT(vt->fs_make_directory);
return vt->fs_make_directory(path);
}
bool al_filename_exists(const char *path)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(path != NULL);
ASSERT(vt->fs_filename_exists);
return vt->fs_filename_exists(path);
}
bool al_remove_filename(const char *path)
{
const ALLEGRO_FS_INTERFACE *vt = al_get_fs_interface();
ASSERT(vt->fs_remove_filename);
ASSERT(path != NULL);
return vt->fs_remove_filename(path);
}
ALLEGRO_FILE *al_open_fs_entry(ALLEGRO_FS_ENTRY *e, const char *mode)
{
ASSERT(e != NULL);
if (e->vtable->fs_open_file)
return e->vtable->fs_open_file(e, mode);
al_set_errno(EINVAL);
return NULL;
}
int al_for_each_fs_entry(ALLEGRO_FS_ENTRY *dir,
int (*callback)(ALLEGRO_FS_ENTRY *dir, void *extra),
void *extra)
{
ALLEGRO_FS_ENTRY *entry;
if (!dir || !al_open_directory(dir)) {
al_set_errno(ENOENT);
return ALLEGRO_FOR_EACH_FS_ENTRY_ERROR;
}
for (entry = al_read_directory(dir); entry; entry = al_read_directory(dir)) {
int result = callback(entry, extra);
if (result == ALLEGRO_FOR_EACH_FS_ENTRY_OK) {
if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
result = al_for_each_fs_entry(entry, callback, extra);
}
}
al_destroy_fs_entry(entry);
if ((result == ALLEGRO_FOR_EACH_FS_ENTRY_STOP) ||
(result == ALLEGRO_FOR_EACH_FS_ENTRY_ERROR)) {
return result;
}
}
return ALLEGRO_FOR_EACH_FS_ENTRY_OK;
}
