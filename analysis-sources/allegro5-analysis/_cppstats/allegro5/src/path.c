#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_vector.h"
#include "allegro5/internal/aintern_path.h"
static ALLEGRO_USTR *get_segment(const ALLEGRO_PATH *path, unsigned i)
{
ALLEGRO_USTR **seg = _al_vector_ref(&path->segments, i);
return *seg;
}
static const char *get_segment_cstr(const ALLEGRO_PATH *path, unsigned i)
{
return al_cstr(get_segment(path, i));
}
static void replace_backslashes(ALLEGRO_USTR *path)
{
al_ustr_find_replace_cstr(path, 0, "\\", "/");
}
static bool parse_path_string(const ALLEGRO_USTR *str, ALLEGRO_PATH *path)
{
ALLEGRO_USTR_INFO dot_info;
ALLEGRO_USTR_INFO dotdot_info;
const ALLEGRO_USTR * dot = al_ref_cstr(&dot_info, ".");
const ALLEGRO_USTR * dotdot = al_ref_cstr(&dotdot_info, "..");
ALLEGRO_USTR *piece = al_ustr_new("");
int pos = 0;
bool on_windows;
#if defined(ALLEGRO_WINDOWS)
on_windows = true;
#else
on_windows = false;
#endif
if (on_windows) {
if (al_ustr_has_prefix_cstr(str, "//")) {
int slash = al_ustr_find_chr(str, 2, '/');
if (slash == -1 || slash == 2) {
goto Error;
}
al_ustr_assign_substr(path->drive, str, pos, slash);
pos = slash;
}
else {
int colon = al_ustr_offset(str, 1);
if (colon > -1 && al_ustr_get(str, colon) == ':') {
al_ustr_assign_substr(path->drive, str, 0, colon + 1);
pos = colon + 1;
}
}
}
for (;;) {
int slash = al_ustr_find_chr(str, pos, '/');
if (slash == -1) {
al_ustr_assign_substr(piece, str, pos, al_ustr_size(str));
if (al_ustr_equal(piece, dot) || al_ustr_equal(piece, dotdot)) {
al_append_path_component(path, al_cstr(piece));
}
else {
al_ustr_assign(path->filename, piece);
}
break;
}
al_ustr_assign_substr(piece, str, pos, slash);
al_append_path_component(path, al_cstr(piece));
pos = slash + 1;
}
al_ustr_free(piece);
return true;
Error:
al_ustr_free(piece);
return false;
}
ALLEGRO_PATH *al_create_path(const char *str)
{
ALLEGRO_PATH *path;
path = al_malloc(sizeof(ALLEGRO_PATH));
if (!path)
return NULL;
path->drive = al_ustr_new("");
path->filename = al_ustr_new("");
_al_vector_init(&path->segments, sizeof(ALLEGRO_USTR *));
path->basename = al_ustr_new("");
path->full_string = al_ustr_new("");
if (str != NULL) {
ALLEGRO_USTR *copy = al_ustr_new(str);
replace_backslashes(copy);
if (!parse_path_string(copy, path)) {
al_destroy_path(path);
path = NULL;
}
al_ustr_free(copy);
}
return path;
}
ALLEGRO_PATH *al_create_path_for_directory(const char *str)
{
ALLEGRO_PATH *path = al_create_path(str);
if (al_ustr_length(path->filename)) {
ALLEGRO_USTR *last = path->filename;
path->filename = al_ustr_new("");
al_append_path_component(path, al_cstr(last));
al_ustr_free(last);
}
return path;
}
ALLEGRO_PATH *al_clone_path(const ALLEGRO_PATH *path)
{
ALLEGRO_PATH *clone;
unsigned int i;
ASSERT(path);
clone = al_create_path(NULL);
if (!clone) {
return NULL;
}
al_ustr_assign(clone->drive, path->drive);
al_ustr_assign(clone->filename, path->filename);
for (i = 0; i < _al_vector_size(&path->segments); i++) {
ALLEGRO_USTR **slot = _al_vector_alloc_back(&clone->segments);
(*slot) = al_ustr_dup(get_segment(path, i));
}
return clone;
}
int al_get_path_num_components(const ALLEGRO_PATH *path)
{
ASSERT(path);
return _al_vector_size(&path->segments);
}
const char *al_get_path_component(const ALLEGRO_PATH *path, int i)
{
ASSERT(path);
ASSERT(i < (int)_al_vector_size(&path->segments));
if (i < 0)
i = _al_vector_size(&path->segments) + i;
ASSERT(i >= 0);
return get_segment_cstr(path, i);
}
void al_replace_path_component(ALLEGRO_PATH *path, int i, const char *s)
{
ASSERT(path);
ASSERT(s);
ASSERT(i < (int)_al_vector_size(&path->segments));
if (i < 0)
i = _al_vector_size(&path->segments) + i;
ASSERT(i >= 0);
al_ustr_assign_cstr(get_segment(path, i), s);
}
void al_remove_path_component(ALLEGRO_PATH *path, int i)
{
ASSERT(path);
ASSERT(i < (int)_al_vector_size(&path->segments));
if (i < 0)
i = _al_vector_size(&path->segments) + i;
ASSERT(i >= 0);
al_ustr_free(get_segment(path, i));
_al_vector_delete_at(&path->segments, i);
}
void al_insert_path_component(ALLEGRO_PATH *path, int i, const char *s)
{
ALLEGRO_USTR **slot;
ASSERT(path);
ASSERT(i <= (int)_al_vector_size(&path->segments));
if (i < 0)
i = _al_vector_size(&path->segments) + i;
ASSERT(i >= 0);
slot = _al_vector_alloc_mid(&path->segments, i);
(*slot) = al_ustr_new(s);
}
const char *al_get_path_tail(const ALLEGRO_PATH *path)
{
ASSERT(path);
if (al_get_path_num_components(path) == 0)
return NULL;
return al_get_path_component(path, -1);
}
void al_drop_path_tail(ALLEGRO_PATH *path)
{
if (al_get_path_num_components(path) > 0) {
al_remove_path_component(path, -1);
}
}
void al_append_path_component(ALLEGRO_PATH *path, const char *s)
{
ALLEGRO_USTR **slot = _al_vector_alloc_back(&path->segments);
(*slot) = al_ustr_new(s);
}
static bool path_is_absolute(const ALLEGRO_PATH *path)
{
return (_al_vector_size(&path->segments) > 0)
&& (al_ustr_size(get_segment(path, 0)) == 0);
}
bool al_join_paths(ALLEGRO_PATH *path, const ALLEGRO_PATH *tail)
{
unsigned i;
ASSERT(path);
ASSERT(tail);
if (path_is_absolute(tail)) {
return false;
}
al_ustr_assign(path->filename, tail->filename);
for (i = 0; i < _al_vector_size(&tail->segments); i++) {
al_append_path_component(path, get_segment_cstr(tail, i));
}
return true;
}
bool al_rebase_path(const ALLEGRO_PATH *head, ALLEGRO_PATH *tail)
{
unsigned i;
ASSERT(head);
ASSERT(tail);
if (path_is_absolute(tail)) {
return false;
}
al_set_path_drive(tail, al_get_path_drive(head));
for (i = 0; i < _al_vector_size(&head->segments); i++) {
al_insert_path_component(tail, i, get_segment_cstr(head, i));
}
return true;
}
static void path_to_ustr(const ALLEGRO_PATH *path, int32_t delim,
ALLEGRO_USTR *str)
{
unsigned i;
al_ustr_assign(str, path->drive);
for (i = 0; i < _al_vector_size(&path->segments); i++) {
al_ustr_append(str, get_segment(path, i));
al_ustr_append_chr(str, delim);
}
al_ustr_append(str, path->filename);
}
const char *al_path_cstr(const ALLEGRO_PATH *path, char delim)
{
return al_cstr(al_path_ustr(path, delim));
}
const ALLEGRO_USTR *al_path_ustr(const ALLEGRO_PATH *path, char delim)
{
path_to_ustr(path, delim, path->full_string);
return path->full_string;
}
void al_destroy_path(ALLEGRO_PATH *path)
{
unsigned i;
if (!path) {
return;
}
if (path->drive) {
al_ustr_free(path->drive);
path->drive = NULL;
}
if (path->filename) {
al_ustr_free(path->filename);
path->filename = NULL;
}
for (i = 0; i < _al_vector_size(&path->segments); i++) {
al_ustr_free(get_segment(path, i));
}
_al_vector_free(&path->segments);
if (path->basename) {
al_ustr_free(path->basename);
path->basename = NULL;
}
if (path->full_string) {
al_ustr_free(path->full_string);
path->full_string = NULL;
}
al_free(path);
}
void al_set_path_drive(ALLEGRO_PATH *path, const char *drive)
{
ASSERT(path);
if (drive)
al_ustr_assign_cstr(path->drive, drive);
else
al_ustr_truncate(path->drive, 0);
}
const char *al_get_path_drive(const ALLEGRO_PATH *path)
{
ASSERT(path);
return al_cstr(path->drive);
}
void al_set_path_filename(ALLEGRO_PATH *path, const char *filename)
{
ASSERT(path);
if (filename)
al_ustr_assign_cstr(path->filename, filename);
else
al_ustr_truncate(path->filename, 0);
}
const char *al_get_path_filename(const ALLEGRO_PATH *path)
{
ASSERT(path);
return al_cstr(path->filename);
}
const char *al_get_path_extension(const ALLEGRO_PATH *path)
{
int pos;
ASSERT(path);
pos = al_ustr_rfind_chr(path->filename, al_ustr_size(path->filename), '.');
if (pos == -1)
pos = al_ustr_size(path->filename);
return al_cstr(path->filename) + pos; 
}
bool al_set_path_extension(ALLEGRO_PATH *path, char const *extension)
{
int dot;
ASSERT(path);
if (al_ustr_size(path->filename) == 0) {
return false;
}
dot = al_ustr_rfind_chr(path->filename, al_ustr_size(path->filename), '.');
if (dot >= 0) {
al_ustr_truncate(path->filename, dot);
}
al_ustr_append_cstr(path->filename, extension);
return true;
}
const char *al_get_path_basename(const ALLEGRO_PATH *path)
{
int dot;
ASSERT(path);
dot = al_ustr_rfind_chr(path->filename, al_ustr_size(path->filename), '.');
if (dot >= 0) {
al_ustr_assign_substr(path->basename, path->filename, 0, dot);
return al_cstr(path->basename);
}
return al_cstr(path->filename);
}
bool al_make_path_canonical(ALLEGRO_PATH *path)
{
unsigned i;
ASSERT(path);
for (i = 0; i < _al_vector_size(&path->segments); ) {
if (strcmp(get_segment_cstr(path, i), ".") == 0)
al_remove_path_component(path, i);
else
i++;
}
if (_al_vector_size(&path->segments) >= 1 &&
al_ustr_size(get_segment(path, 0)) == 0)
{
while (_al_vector_size(&path->segments) >= 2 &&
strcmp(get_segment_cstr(path, 1), "..") == 0)
{
al_remove_path_component(path, 1);
}
}
return true;
}
