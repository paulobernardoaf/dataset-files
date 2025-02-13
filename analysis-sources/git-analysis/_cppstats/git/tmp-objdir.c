#include "cache.h"
#include "tmp-objdir.h"
#include "dir.h"
#include "sigchain.h"
#include "string-list.h"
#include "strbuf.h"
#include "argv-array.h"
#include "quote.h"
#include "object-store.h"
struct tmp_objdir {
struct strbuf path;
struct argv_array env;
};
static struct tmp_objdir *the_tmp_objdir;
static void tmp_objdir_free(struct tmp_objdir *t)
{
strbuf_release(&t->path);
argv_array_clear(&t->env);
free(t);
}
static int tmp_objdir_destroy_1(struct tmp_objdir *t, int on_signal)
{
int err;
if (!t)
return 0;
if (t == the_tmp_objdir)
the_tmp_objdir = NULL;
err = remove_dir_recursively(&t->path, 0);
if (!on_signal)
tmp_objdir_free(t);
return err;
}
int tmp_objdir_destroy(struct tmp_objdir *t)
{
return tmp_objdir_destroy_1(t, 0);
}
static void remove_tmp_objdir(void)
{
tmp_objdir_destroy(the_tmp_objdir);
}
static void remove_tmp_objdir_on_signal(int signo)
{
tmp_objdir_destroy_1(the_tmp_objdir, 1);
sigchain_pop(signo);
raise(signo);
}
static void env_append(struct argv_array *env, const char *key, const char *val)
{
struct strbuf quoted = STRBUF_INIT;
const char *old;
if (*val == '"' || strchr(val, PATH_SEP)) {
strbuf_addch(&quoted, '"');
quote_c_style(val, &quoted, NULL, 1);
strbuf_addch(&quoted, '"');
val = quoted.buf;
}
old = getenv(key);
if (!old)
argv_array_pushf(env, "%s=%s", key, val);
else
argv_array_pushf(env, "%s=%s%c%s", key, old, PATH_SEP, val);
strbuf_release(&quoted);
}
static void env_replace(struct argv_array *env, const char *key, const char *val)
{
argv_array_pushf(env, "%s=%s", key, val);
}
static int setup_tmp_objdir(const char *root)
{
char *path;
int ret = 0;
path = xstrfmt("%s/pack", root);
ret = mkdir(path, 0777);
free(path);
return ret;
}
struct tmp_objdir *tmp_objdir_create(void)
{
static int installed_handlers;
struct tmp_objdir *t;
if (the_tmp_objdir)
BUG("only one tmp_objdir can be used at a time");
t = xmalloc(sizeof(*t));
strbuf_init(&t->path, 0);
argv_array_init(&t->env);
strbuf_addf(&t->path, "%s/incoming-XXXXXX", get_object_directory());
strbuf_grow(&t->path, 1024);
if (!mkdtemp(t->path.buf)) {
tmp_objdir_free(t);
return NULL;
}
the_tmp_objdir = t;
if (!installed_handlers) {
atexit(remove_tmp_objdir);
sigchain_push_common(remove_tmp_objdir_on_signal);
installed_handlers++;
}
if (setup_tmp_objdir(t->path.buf)) {
tmp_objdir_destroy(t);
return NULL;
}
env_append(&t->env, ALTERNATE_DB_ENVIRONMENT,
absolute_path(get_object_directory()));
env_replace(&t->env, DB_ENVIRONMENT, absolute_path(t->path.buf));
env_replace(&t->env, GIT_QUARANTINE_ENVIRONMENT,
absolute_path(t->path.buf));
return t;
}
static int pack_copy_priority(const char *name)
{
if (!starts_with(name, "pack"))
return 0;
if (ends_with(name, ".keep"))
return 1;
if (ends_with(name, ".pack"))
return 2;
if (ends_with(name, ".idx"))
return 3;
return 4;
}
static int pack_copy_cmp(const char *a, const char *b)
{
return pack_copy_priority(a) - pack_copy_priority(b);
}
static int read_dir_paths(struct string_list *out, const char *path)
{
DIR *dh;
struct dirent *de;
dh = opendir(path);
if (!dh)
return -1;
while ((de = readdir(dh)))
if (de->d_name[0] != '.')
string_list_append(out, de->d_name);
closedir(dh);
return 0;
}
static int migrate_paths(struct strbuf *src, struct strbuf *dst);
static int migrate_one(struct strbuf *src, struct strbuf *dst)
{
struct stat st;
if (stat(src->buf, &st) < 0)
return -1;
if (S_ISDIR(st.st_mode)) {
if (!mkdir(dst->buf, 0777)) {
if (adjust_shared_perm(dst->buf))
return -1;
} else if (errno != EEXIST)
return -1;
return migrate_paths(src, dst);
}
return finalize_object_file(src->buf, dst->buf);
}
static int migrate_paths(struct strbuf *src, struct strbuf *dst)
{
size_t src_len = src->len, dst_len = dst->len;
struct string_list paths = STRING_LIST_INIT_DUP;
int i;
int ret = 0;
if (read_dir_paths(&paths, src->buf) < 0)
return -1;
paths.cmp = pack_copy_cmp;
string_list_sort(&paths);
for (i = 0; i < paths.nr; i++) {
const char *name = paths.items[i].string;
strbuf_addf(src, "/%s", name);
strbuf_addf(dst, "/%s", name);
ret |= migrate_one(src, dst);
strbuf_setlen(src, src_len);
strbuf_setlen(dst, dst_len);
}
string_list_clear(&paths, 0);
return ret;
}
int tmp_objdir_migrate(struct tmp_objdir *t)
{
struct strbuf src = STRBUF_INIT, dst = STRBUF_INIT;
int ret;
if (!t)
return 0;
strbuf_addbuf(&src, &t->path);
strbuf_addstr(&dst, get_object_directory());
ret = migrate_paths(&src, &dst);
strbuf_release(&src);
strbuf_release(&dst);
tmp_objdir_destroy(t);
return ret;
}
const char **tmp_objdir_env(const struct tmp_objdir *t)
{
if (!t)
return NULL;
return t->env.argv;
}
void tmp_objdir_add_as_alternate(const struct tmp_objdir *t)
{
add_to_alternates_memory(t->path.buf);
}
