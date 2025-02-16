#include "cache.h"
int is_directory(const char *path)
{
struct stat st;
return (!stat(path, &st) && S_ISDIR(st.st_mode));
}
static void strip_last_component(struct strbuf *path)
{
size_t offset = offset_1st_component(path->buf);
size_t len = path->len;
while (offset < len && !is_dir_sep(path->buf[len - 1]))
len--;
while (offset < len && is_dir_sep(path->buf[len - 1]))
len--;
strbuf_setlen(path, len);
}
static void get_next_component(struct strbuf *next, struct strbuf *remaining)
{
char *start = NULL;
char *end = NULL;
strbuf_reset(next);
for (start = remaining->buf; is_dir_sep(*start); start++)
; 
for (end = start; *end && !is_dir_sep(*end); end++)
; 
strbuf_add(next, start, end - start);
strbuf_remove(remaining, 0, end - remaining->buf);
}
static void get_root_part(struct strbuf *resolved, struct strbuf *remaining)
{
int offset = offset_1st_component(remaining->buf);
strbuf_reset(resolved);
strbuf_add(resolved, remaining->buf, offset);
#if defined(GIT_WINDOWS_NATIVE)
convert_slashes(resolved->buf);
#endif
strbuf_remove(remaining, 0, offset);
}
#if !defined(MAXSYMLINKS)
#define MAXSYMLINKS 32
#endif
char *strbuf_realpath(struct strbuf *resolved, const char *path,
int die_on_error)
{
struct strbuf remaining = STRBUF_INIT;
struct strbuf next = STRBUF_INIT;
struct strbuf symlink = STRBUF_INIT;
char *retval = NULL;
int num_symlinks = 0;
struct stat st;
if (!*path) {
if (die_on_error)
die("The empty string is not a valid path");
else
goto error_out;
}
strbuf_addstr(&remaining, path);
get_root_part(resolved, &remaining);
if (!resolved->len) {
if (strbuf_getcwd(resolved)) {
if (die_on_error)
die_errno("unable to get current working directory");
else
goto error_out;
}
}
while (remaining.len > 0) {
get_next_component(&next, &remaining);
if (next.len == 0) {
continue; 
} else if (next.len == 1 && !strcmp(next.buf, ".")) {
continue; 
} else if (next.len == 2 && !strcmp(next.buf, "..")) {
strip_last_component(resolved);
continue;
}
if (!is_dir_sep(resolved->buf[resolved->len - 1]))
strbuf_addch(resolved, '/');
strbuf_addbuf(resolved, &next);
if (lstat(resolved->buf, &st)) {
if (errno != ENOENT || remaining.len) {
if (die_on_error)
die_errno("Invalid path '%s'",
resolved->buf);
else
goto error_out;
}
} else if (S_ISLNK(st.st_mode)) {
ssize_t len;
strbuf_reset(&symlink);
if (num_symlinks++ > MAXSYMLINKS) {
errno = ELOOP;
if (die_on_error)
die("More than %d nested symlinks "
"on path '%s'", MAXSYMLINKS, path);
else
goto error_out;
}
len = strbuf_readlink(&symlink, resolved->buf,
st.st_size);
if (len < 0) {
if (die_on_error)
die_errno("Invalid symlink '%s'",
resolved->buf);
else
goto error_out;
}
if (is_absolute_path(symlink.buf)) {
get_root_part(resolved, &symlink);
} else {
strip_last_component(resolved);
}
if (remaining.len) {
strbuf_addch(&symlink, '/');
strbuf_addbuf(&symlink, &remaining);
}
strbuf_swap(&symlink, &remaining);
}
}
retval = resolved->buf;
error_out:
strbuf_release(&remaining);
strbuf_release(&next);
strbuf_release(&symlink);
if (!retval)
strbuf_reset(resolved);
return retval;
}
char *real_pathdup(const char *path, int die_on_error)
{
struct strbuf realpath = STRBUF_INIT;
char *retval = NULL;
if (strbuf_realpath(&realpath, path, die_on_error))
retval = strbuf_detach(&realpath, NULL);
strbuf_release(&realpath);
return retval;
}
const char *absolute_path(const char *path)
{
static struct strbuf sb = STRBUF_INIT;
strbuf_reset(&sb);
strbuf_add_absolute_path(&sb, path);
return sb.buf;
}
char *absolute_pathdup(const char *path)
{
struct strbuf sb = STRBUF_INIT;
strbuf_add_absolute_path(&sb, path);
return strbuf_detach(&sb, NULL);
}
char *prefix_filename(const char *pfx, const char *arg)
{
struct strbuf path = STRBUF_INIT;
size_t pfx_len = pfx ? strlen(pfx) : 0;
if (!pfx_len)
; 
else if (is_absolute_path(arg))
pfx_len = 0;
else
strbuf_add(&path, pfx, pfx_len);
strbuf_addstr(&path, arg);
#if defined(GIT_WINDOWS_NATIVE)
convert_slashes(path.buf + pfx_len);
#endif
return strbuf_detach(&path, NULL);
}
