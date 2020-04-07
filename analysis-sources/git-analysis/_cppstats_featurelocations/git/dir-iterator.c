#include "cache.h"
#include "dir.h"
#include "iterator.h"
#include "dir-iterator.h"

struct dir_iterator_level {
DIR *dir;





size_t prefix_len;
};






struct dir_iterator_int {
struct dir_iterator base;








size_t levels_nr;


size_t levels_alloc;





struct dir_iterator_level *levels;


unsigned int flags;
};







static int push_level(struct dir_iterator_int *iter)
{
struct dir_iterator_level *level;

ALLOC_GROW(iter->levels, iter->levels_nr + 1, iter->levels_alloc);
level = &iter->levels[iter->levels_nr++];

if (!is_dir_sep(iter->base.path.buf[iter->base.path.len - 1]))
strbuf_addch(&iter->base.path, '/');
level->prefix_len = iter->base.path.len;

level->dir = opendir(iter->base.path.buf);
if (!level->dir) {
int saved_errno = errno;
if (errno != ENOENT) {
warning_errno("error opening directory '%s'",
iter->base.path.buf);
}
iter->levels_nr--;
errno = saved_errno;
return -1;
}

return 0;
}





static int pop_level(struct dir_iterator_int *iter)
{
struct dir_iterator_level *level =
&iter->levels[iter->levels_nr - 1];

if (level->dir && closedir(level->dir))
warning_errno("error closing directory '%s'",
iter->base.path.buf);
level->dir = NULL;

return --iter->levels_nr;
}






static int prepare_next_entry_data(struct dir_iterator_int *iter,
struct dirent *de)
{
int err, saved_errno;

strbuf_addstr(&iter->base.path, de->d_name);




iter->base.relative_path = iter->base.path.buf +
iter->levels[0].prefix_len;
iter->base.basename = iter->base.path.buf +
iter->levels[iter->levels_nr - 1].prefix_len;

if (iter->flags & DIR_ITERATOR_FOLLOW_SYMLINKS)
err = stat(iter->base.path.buf, &iter->base.st);
else
err = lstat(iter->base.path.buf, &iter->base.st);

saved_errno = errno;
if (err && errno != ENOENT)
warning_errno("failed to stat '%s'", iter->base.path.buf);

errno = saved_errno;
return err;
}

int dir_iterator_advance(struct dir_iterator *dir_iterator)
{
struct dir_iterator_int *iter =
(struct dir_iterator_int *)dir_iterator;

if (S_ISDIR(iter->base.st.st_mode) && push_level(iter)) {
if (errno != ENOENT && iter->flags & DIR_ITERATOR_PEDANTIC)
goto error_out;
if (iter->levels_nr == 0)
goto error_out;
}


while (1) {
struct dirent *de;
struct dir_iterator_level *level =
&iter->levels[iter->levels_nr - 1];

strbuf_setlen(&iter->base.path, level->prefix_len);
errno = 0;
de = readdir(level->dir);

if (!de) {
if (errno) {
warning_errno("error reading directory '%s'",
iter->base.path.buf);
if (iter->flags & DIR_ITERATOR_PEDANTIC)
goto error_out;
} else if (pop_level(iter) == 0) {
return dir_iterator_abort(dir_iterator);
}
continue;
}

if (is_dot_or_dotdot(de->d_name))
continue;

if (prepare_next_entry_data(iter, de)) {
if (errno != ENOENT && iter->flags & DIR_ITERATOR_PEDANTIC)
goto error_out;
continue;
}

return ITER_OK;
}

error_out:
dir_iterator_abort(dir_iterator);
return ITER_ERROR;
}

int dir_iterator_abort(struct dir_iterator *dir_iterator)
{
struct dir_iterator_int *iter = (struct dir_iterator_int *)dir_iterator;

for (; iter->levels_nr; iter->levels_nr--) {
struct dir_iterator_level *level =
&iter->levels[iter->levels_nr - 1];

if (level->dir && closedir(level->dir)) {
int saved_errno = errno;
strbuf_setlen(&iter->base.path, level->prefix_len);
errno = saved_errno;
warning_errno("error closing directory '%s'",
iter->base.path.buf);
}
}

free(iter->levels);
strbuf_release(&iter->base.path);
free(iter);
return ITER_DONE;
}

struct dir_iterator *dir_iterator_begin(const char *path, unsigned int flags)
{
struct dir_iterator_int *iter = xcalloc(1, sizeof(*iter));
struct dir_iterator *dir_iterator = &iter->base;
int saved_errno;

strbuf_init(&iter->base.path, PATH_MAX);
strbuf_addstr(&iter->base.path, path);

ALLOC_GROW(iter->levels, 10, iter->levels_alloc);
iter->levels_nr = 0;
iter->flags = flags;





if (stat(iter->base.path.buf, &iter->base.st) < 0) {
saved_errno = errno;
goto error_out;
}

if (!S_ISDIR(iter->base.st.st_mode)) {
saved_errno = ENOTDIR;
goto error_out;
}

return dir_iterator;

error_out:
dir_iterator_abort(dir_iterator);
errno = saved_errno;
return NULL;
}
