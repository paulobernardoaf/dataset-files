#include "cache.h"
#include "repository.h"
#include "config.h"
#include "dir.h"
#include "string-list.h"
#include "chdir-notify.h"
#include "promisor-remote.h"

static int inside_git_dir = -1;
static int inside_work_tree = -1;
static int work_tree_config_is_bogus;

static struct startup_info the_startup_info;
struct startup_info *startup_info = &the_startup_info;













static int abspath_part_inside_repo(char *path)
{
size_t len;
size_t wtlen;
char *path0;
int off;
const char *work_tree = get_git_work_tree();
struct strbuf realpath = STRBUF_INIT;

if (!work_tree)
return -1;
wtlen = strlen(work_tree);
len = strlen(path);
off = offset_1st_component(path);


if (wtlen <= len && !fspathncmp(path, work_tree, wtlen)) {
if (path[wtlen] == '/') {
memmove(path, path + wtlen + 1, len - wtlen);
return 0;
} else if (path[wtlen - 1] == '/' || path[wtlen] == '\0') {

memmove(path, path + wtlen, len - wtlen + 1);
return 0;
}

off = wtlen;
}
path0 = path;
path += off;


while (*path) {
path++;
if (*path == '/') {
*path = '\0';
strbuf_realpath(&realpath, path0, 1);
if (fspathcmp(realpath.buf, work_tree) == 0) {
memmove(path0, path + 1, len - (path - path0));
strbuf_release(&realpath);
return 0;
}
*path = '/';
}
}


strbuf_realpath(&realpath, path0, 1);
if (fspathcmp(realpath.buf, work_tree) == 0) {
*path0 = '\0';
strbuf_release(&realpath);
return 0;
}

strbuf_release(&realpath);
return -1;
}












char *prefix_path_gently(const char *prefix, int len,
int *remaining_prefix, const char *path)
{
const char *orig = path;
char *sanitized;
if (is_absolute_path(orig)) {
sanitized = xmallocz(strlen(path));
if (remaining_prefix)
*remaining_prefix = 0;
if (normalize_path_copy_len(sanitized, path, remaining_prefix)) {
free(sanitized);
return NULL;
}
if (abspath_part_inside_repo(sanitized)) {
free(sanitized);
return NULL;
}
} else {
sanitized = xstrfmt("%.*s%s", len, len ? prefix : "", path);
if (remaining_prefix)
*remaining_prefix = len;
if (normalize_path_copy_len(sanitized, sanitized, remaining_prefix)) {
free(sanitized);
return NULL;
}
}
return sanitized;
}

char *prefix_path(const char *prefix, int len, const char *path)
{
char *r = prefix_path_gently(prefix, len, NULL, path);
if (!r) {
const char *hint_path = get_git_work_tree();
if (!hint_path)
hint_path = get_git_dir();
die(_("'%s' is outside repository at '%s'"), path,
absolute_path(hint_path));
}
return r;
}

int path_inside_repo(const char *prefix, const char *path)
{
int len = prefix ? strlen(prefix) : 0;
char *r = prefix_path_gently(prefix, len, NULL, path);
if (r) {
free(r);
return 1;
}
return 0;
}

int check_filename(const char *prefix, const char *arg)
{
char *to_free = NULL;
struct stat st;

if (skip_prefix(arg, ":/", &arg)) {
if (!*arg) 
return 1;
prefix = NULL;
} else if (skip_prefix(arg, ":!", &arg) ||
skip_prefix(arg, ":^", &arg)) {
if (!*arg) 
return 1;
}

if (prefix)
arg = to_free = prefix_filename(prefix, arg);

if (!lstat(arg, &st)) {
free(to_free);
return 1; 
}
if (is_missing_file_error(errno)) {
free(to_free);
return 0; 
}
die_errno(_("failed to stat '%s'"), arg);
}

static void NORETURN die_verify_filename(struct repository *r,
const char *prefix,
const char *arg,
int diagnose_misspelt_rev)
{
if (!diagnose_misspelt_rev)
die(_("%s: no such path in the working tree.\n"
"Use 'git <command> -- <path>...' to specify paths that do not exist locally."),
arg);






if (!(arg[0] == ':' && !isalnum(arg[1])))
maybe_die_on_misspelt_object_name(r, arg, prefix);


die(_("ambiguous argument '%s': unknown revision or path not in the working tree.\n"
"Use '--' to separate paths from revisions, like this:\n"
"'git <command> [<revision>...] -- [<file>...]'"), arg);

}






static int looks_like_pathspec(const char *arg)
{
const char *p;
int escaped = 0;








for (p = arg; *p; p++) {
if (escaped) {
escaped = 0;
} else if (is_glob_special(*p)) {
if (*p == '\\')
escaped = 1;
else
return 1;
}
}


if (starts_with(arg, ":("))
return 1;

return 0;
}





















void verify_filename(const char *prefix,
const char *arg,
int diagnose_misspelt_rev)
{
if (*arg == '-')
die(_("option '%s' must come before non-option arguments"), arg);
if (looks_like_pathspec(arg) || check_filename(prefix, arg))
return;
die_verify_filename(the_repository, prefix, arg, diagnose_misspelt_rev);
}






void verify_non_filename(const char *prefix, const char *arg)
{
if (!is_inside_work_tree() || is_inside_git_dir())
return;
if (*arg == '-')
return; 
if (!check_filename(prefix, arg))
return;
die(_("ambiguous argument '%s': both revision and filename\n"
"Use '--' to separate paths from revisions, like this:\n"
"'git <command> [<revision>...] -- [<file>...]'"), arg);
}

int get_common_dir(struct strbuf *sb, const char *gitdir)
{
const char *git_env_common_dir = getenv(GIT_COMMON_DIR_ENVIRONMENT);
if (git_env_common_dir) {
strbuf_addstr(sb, git_env_common_dir);
return 1;
} else {
return get_common_dir_noenv(sb, gitdir);
}
}

int get_common_dir_noenv(struct strbuf *sb, const char *gitdir)
{
struct strbuf data = STRBUF_INIT;
struct strbuf path = STRBUF_INIT;
int ret = 0;

strbuf_addf(&path, "%s/commondir", gitdir);
if (file_exists(path.buf)) {
if (strbuf_read_file(&data, path.buf, 0) <= 0)
die_errno(_("failed to read %s"), path.buf);
while (data.len && (data.buf[data.len - 1] == '\n' ||
data.buf[data.len - 1] == '\r'))
data.len--;
data.buf[data.len] = '\0';
strbuf_reset(&path);
if (!is_absolute_path(data.buf))
strbuf_addf(&path, "%s/", gitdir);
strbuf_addbuf(&path, &data);
strbuf_add_real_path(sb, path.buf);
ret = 1;
} else {
strbuf_addstr(sb, gitdir);
}

strbuf_release(&data);
strbuf_release(&path);
return ret;
}












int is_git_directory(const char *suspect)
{
struct strbuf path = STRBUF_INIT;
int ret = 0;
size_t len;


strbuf_addstr(&path, suspect);
strbuf_complete(&path, '/');
strbuf_addstr(&path, "HEAD");
if (validate_headref(path.buf))
goto done;

strbuf_reset(&path);
get_common_dir(&path, suspect);
len = path.len;


if (getenv(DB_ENVIRONMENT)) {
if (access(getenv(DB_ENVIRONMENT), X_OK))
goto done;
}
else {
strbuf_setlen(&path, len);
strbuf_addstr(&path, "/objects");
if (access(path.buf, X_OK))
goto done;
}

strbuf_setlen(&path, len);
strbuf_addstr(&path, "/refs");
if (access(path.buf, X_OK))
goto done;

ret = 1;
done:
strbuf_release(&path);
return ret;
}

int is_nonbare_repository_dir(struct strbuf *path)
{
int ret = 0;
int gitfile_error;
size_t orig_path_len = path->len;
assert(orig_path_len != 0);
strbuf_complete(path, '/');
strbuf_addstr(path, ".git");
if (read_gitfile_gently(path->buf, &gitfile_error) || is_git_directory(path->buf))
ret = 1;
if (gitfile_error == READ_GITFILE_ERR_OPEN_FAILED ||
gitfile_error == READ_GITFILE_ERR_READ_FAILED)
ret = 1;
strbuf_setlen(path, orig_path_len);
return ret;
}

int is_inside_git_dir(void)
{
if (inside_git_dir < 0)
inside_git_dir = is_inside_dir(get_git_dir());
return inside_git_dir;
}

int is_inside_work_tree(void)
{
if (inside_work_tree < 0)
inside_work_tree = is_inside_dir(get_git_work_tree());
return inside_work_tree;
}

void setup_work_tree(void)
{
const char *work_tree;
static int initialized = 0;

if (initialized)
return;

if (work_tree_config_is_bogus)
die(_("unable to set up work tree using invalid config"));

work_tree = get_git_work_tree();
if (!work_tree || chdir_notify(work_tree))
die(_("this operation must be run in a work tree"));





if (getenv(GIT_WORK_TREE_ENVIRONMENT))
setenv(GIT_WORK_TREE_ENVIRONMENT, ".", 1);

initialized = 1;
}

static int read_worktree_config(const char *var, const char *value, void *vdata)
{
struct repository_format *data = vdata;

if (strcmp(var, "core.bare") == 0) {
data->is_bare = git_config_bool(var, value);
} else if (strcmp(var, "core.worktree") == 0) {
if (!value)
return config_error_nonbool(var);
free(data->work_tree);
data->work_tree = xstrdup(value);
}
return 0;
}

static int check_repo_format(const char *var, const char *value, void *vdata)
{
struct repository_format *data = vdata;
const char *ext;

if (strcmp(var, "core.repositoryformatversion") == 0)
data->version = git_config_int(var, value);
else if (skip_prefix(var, "extensions.", &ext)) {





if (!strcmp(ext, "noop"))
;
else if (!strcmp(ext, "preciousobjects"))
data->precious_objects = git_config_bool(var, value);
else if (!strcmp(ext, "partialclone")) {
if (!value)
return config_error_nonbool(var);
data->partial_clone = xstrdup(value);
} else if (!strcmp(ext, "worktreeconfig"))
data->worktree_config = git_config_bool(var, value);
else
string_list_append(&data->unknown_extensions, ext);
}

return read_worktree_config(var, value, vdata);
}

static int check_repository_format_gently(const char *gitdir, struct repository_format *candidate, int *nongit_ok)
{
struct strbuf sb = STRBUF_INIT;
struct strbuf err = STRBUF_INIT;
int has_common;

has_common = get_common_dir(&sb, gitdir);
strbuf_addstr(&sb, "/config");
read_repository_format(candidate, sb.buf);
strbuf_release(&sb);






if (candidate->version < 0)
return 0;

if (verify_repository_format(candidate, &err) < 0) {
if (nongit_ok) {
warning("%s", err.buf);
strbuf_release(&err);
*nongit_ok = -1;
return -1;
}
die("%s", err.buf);
}

repository_format_precious_objects = candidate->precious_objects;
set_repository_format_partial_clone(candidate->partial_clone);
repository_format_worktree_config = candidate->worktree_config;
string_list_clear(&candidate->unknown_extensions, 0);

if (repository_format_worktree_config) {




strbuf_addf(&sb, "%s/config.worktree", gitdir);
git_config_from_file(read_worktree_config, sb.buf, candidate);
strbuf_release(&sb);
has_common = 0;
}

if (!has_common) {
if (candidate->is_bare != -1) {
is_bare_repository_cfg = candidate->is_bare;
if (is_bare_repository_cfg == 1)
inside_work_tree = -1;
}
if (candidate->work_tree) {
free(git_work_tree_cfg);
git_work_tree_cfg = xstrdup(candidate->work_tree);
inside_work_tree = -1;
}
}

return 0;
}

static void init_repository_format(struct repository_format *format)
{
const struct repository_format fresh = REPOSITORY_FORMAT_INIT;

memcpy(format, &fresh, sizeof(fresh));
}

int read_repository_format(struct repository_format *format, const char *path)
{
clear_repository_format(format);
git_config_from_file(check_repo_format, path, format);
if (format->version == -1)
clear_repository_format(format);
return format->version;
}

void clear_repository_format(struct repository_format *format)
{
string_list_clear(&format->unknown_extensions, 0);
free(format->work_tree);
free(format->partial_clone);
init_repository_format(format);
}

int verify_repository_format(const struct repository_format *format,
struct strbuf *err)
{
if (GIT_REPO_VERSION_READ < format->version) {
strbuf_addf(err, _("Expected git repo version <= %d, found %d"),
GIT_REPO_VERSION_READ, format->version);
return -1;
}

if (format->version >= 1 && format->unknown_extensions.nr) {
int i;

strbuf_addstr(err, _("unknown repository extensions found:"));

for (i = 0; i < format->unknown_extensions.nr; i++)
strbuf_addf(err, "\n\t%s",
format->unknown_extensions.items[i].string);
return -1;
}

return 0;
}

void read_gitfile_error_die(int error_code, const char *path, const char *dir)
{
switch (error_code) {
case READ_GITFILE_ERR_STAT_FAILED:
case READ_GITFILE_ERR_NOT_A_FILE:

break;
case READ_GITFILE_ERR_OPEN_FAILED:
die_errno(_("error opening '%s'"), path);
case READ_GITFILE_ERR_TOO_LARGE:
die(_("too large to be a .git file: '%s'"), path);
case READ_GITFILE_ERR_READ_FAILED:
die(_("error reading %s"), path);
case READ_GITFILE_ERR_INVALID_FORMAT:
die(_("invalid gitfile format: %s"), path);
case READ_GITFILE_ERR_NO_PATH:
die(_("no path in gitfile: %s"), path);
case READ_GITFILE_ERR_NOT_A_REPO:
die(_("not a git repository: %s"), dir);
default:
BUG("unknown error code");
}
}











const char *read_gitfile_gently(const char *path, int *return_error_code)
{
const int max_file_size = 1 << 20; 
int error_code = 0;
char *buf = NULL;
char *dir = NULL;
const char *slash;
struct stat st;
int fd;
ssize_t len;
static struct strbuf realpath = STRBUF_INIT;

if (stat(path, &st)) {

error_code = READ_GITFILE_ERR_STAT_FAILED;
goto cleanup_return;
}
if (!S_ISREG(st.st_mode)) {
error_code = READ_GITFILE_ERR_NOT_A_FILE;
goto cleanup_return;
}
if (st.st_size > max_file_size) {
error_code = READ_GITFILE_ERR_TOO_LARGE;
goto cleanup_return;
}
fd = open(path, O_RDONLY);
if (fd < 0) {
error_code = READ_GITFILE_ERR_OPEN_FAILED;
goto cleanup_return;
}
buf = xmallocz(st.st_size);
len = read_in_full(fd, buf, st.st_size);
close(fd);
if (len != st.st_size) {
error_code = READ_GITFILE_ERR_READ_FAILED;
goto cleanup_return;
}
if (!starts_with(buf, "gitdir: ")) {
error_code = READ_GITFILE_ERR_INVALID_FORMAT;
goto cleanup_return;
}
while (buf[len - 1] == '\n' || buf[len - 1] == '\r')
len--;
if (len < 9) {
error_code = READ_GITFILE_ERR_NO_PATH;
goto cleanup_return;
}
buf[len] = '\0';
dir = buf + 8;

if (!is_absolute_path(dir) && (slash = strrchr(path, '/'))) {
size_t pathlen = slash+1 - path;
dir = xstrfmt("%.*s%.*s", (int)pathlen, path,
(int)(len - 8), buf + 8);
free(buf);
buf = dir;
}
if (!is_git_directory(dir)) {
error_code = READ_GITFILE_ERR_NOT_A_REPO;
goto cleanup_return;
}

strbuf_realpath(&realpath, dir, 1);
path = realpath.buf;

cleanup_return:
if (return_error_code)
*return_error_code = error_code;
else if (error_code)
read_gitfile_error_die(error_code, path, dir);

free(buf);
return error_code ? NULL : path;
}

static const char *setup_explicit_git_dir(const char *gitdirenv,
struct strbuf *cwd,
struct repository_format *repo_fmt,
int *nongit_ok)
{
const char *work_tree_env = getenv(GIT_WORK_TREE_ENVIRONMENT);
const char *worktree;
char *gitfile;
int offset;

if (PATH_MAX - 40 < strlen(gitdirenv))
die(_("'$%s' too big"), GIT_DIR_ENVIRONMENT);

gitfile = (char*)read_gitfile(gitdirenv);
if (gitfile) {
gitfile = xstrdup(gitfile);
gitdirenv = gitfile;
}

if (!is_git_directory(gitdirenv)) {
if (nongit_ok) {
*nongit_ok = 1;
free(gitfile);
return NULL;
}
die(_("not a git repository: '%s'"), gitdirenv);
}

if (check_repository_format_gently(gitdirenv, repo_fmt, nongit_ok)) {
free(gitfile);
return NULL;
}


if (work_tree_env)
set_git_work_tree(work_tree_env);
else if (is_bare_repository_cfg > 0) {
if (git_work_tree_cfg) {

warning("core.bare and core.worktree do not make sense");
work_tree_config_is_bogus = 1;
}


set_git_dir(gitdirenv, 0);
free(gitfile);
return NULL;
}
else if (git_work_tree_cfg) { 
if (is_absolute_path(git_work_tree_cfg))
set_git_work_tree(git_work_tree_cfg);
else {
char *core_worktree;
if (chdir(gitdirenv))
die_errno(_("cannot chdir to '%s'"), gitdirenv);
if (chdir(git_work_tree_cfg))
die_errno(_("cannot chdir to '%s'"), git_work_tree_cfg);
core_worktree = xgetcwd();
if (chdir(cwd->buf))
die_errno(_("cannot come back to cwd"));
set_git_work_tree(core_worktree);
free(core_worktree);
}
}
else if (!git_env_bool(GIT_IMPLICIT_WORK_TREE_ENVIRONMENT, 1)) {

set_git_dir(gitdirenv, 0);
free(gitfile);
return NULL;
}
else 
set_git_work_tree(".");


worktree = get_git_work_tree();


if (!strcmp(cwd->buf, worktree)) { 
set_git_dir(gitdirenv, 0);
free(gitfile);
return NULL;
}

offset = dir_inside_of(cwd->buf, worktree);
if (offset >= 0) { 
set_git_dir(gitdirenv, 1);
if (chdir(worktree))
die_errno(_("cannot chdir to '%s'"), worktree);
strbuf_addch(cwd, '/');
free(gitfile);
return cwd->buf + offset;
}


set_git_dir(gitdirenv, 0);
free(gitfile);
return NULL;
}

static const char *setup_discovered_git_dir(const char *gitdir,
struct strbuf *cwd, int offset,
struct repository_format *repo_fmt,
int *nongit_ok)
{
if (check_repository_format_gently(gitdir, repo_fmt, nongit_ok))
return NULL;


if (getenv(GIT_WORK_TREE_ENVIRONMENT) || git_work_tree_cfg) {
char *to_free = NULL;
const char *ret;

if (offset != cwd->len && !is_absolute_path(gitdir))
gitdir = to_free = real_pathdup(gitdir, 1);
if (chdir(cwd->buf))
die_errno(_("cannot come back to cwd"));
ret = setup_explicit_git_dir(gitdir, cwd, repo_fmt, nongit_ok);
free(to_free);
return ret;
}


if (is_bare_repository_cfg > 0) {
set_git_dir(gitdir, (offset != cwd->len));
if (chdir(cwd->buf))
die_errno(_("cannot come back to cwd"));
return NULL;
}


set_git_work_tree(".");
if (strcmp(gitdir, DEFAULT_GIT_DIR_ENVIRONMENT))
set_git_dir(gitdir, 0);
inside_git_dir = 0;
inside_work_tree = 1;
if (offset >= cwd->len)
return NULL;


if (offset != offset_1st_component(cwd->buf))
offset++;

strbuf_addch(cwd, '/');
return cwd->buf + offset;
}


static const char *setup_bare_git_dir(struct strbuf *cwd, int offset,
struct repository_format *repo_fmt,
int *nongit_ok)
{
int root_len;

if (check_repository_format_gently(".", repo_fmt, nongit_ok))
return NULL;

setenv(GIT_IMPLICIT_WORK_TREE_ENVIRONMENT, "0", 1);


if (getenv(GIT_WORK_TREE_ENVIRONMENT) || git_work_tree_cfg) {
static const char *gitdir;

gitdir = offset == cwd->len ? "." : xmemdupz(cwd->buf, offset);
if (chdir(cwd->buf))
die_errno(_("cannot come back to cwd"));
return setup_explicit_git_dir(gitdir, cwd, repo_fmt, nongit_ok);
}

inside_git_dir = 1;
inside_work_tree = 0;
if (offset != cwd->len) {
if (chdir(cwd->buf))
die_errno(_("cannot come back to cwd"));
root_len = offset_1st_component(cwd->buf);
strbuf_setlen(cwd, offset > root_len ? offset : root_len);
set_git_dir(cwd->buf, 0);
}
else
set_git_dir(".", 0);
return NULL;
}

static dev_t get_device_or_die(const char *path, const char *prefix, int prefix_len)
{
struct stat buf;
if (stat(path, &buf)) {
die_errno(_("failed to stat '%*s%s%s'"),
prefix_len,
prefix ? prefix : "",
prefix ? "/" : "", path);
}
return buf.st_dev;
}








static int canonicalize_ceiling_entry(struct string_list_item *item,
void *cb_data)
{
int *empty_entry_found = cb_data;
char *ceil = item->string;

if (!*ceil) {
*empty_entry_found = 1;
return 0;
} else if (!is_absolute_path(ceil)) {
return 0;
} else if (*empty_entry_found) {

return 1;
} else {
char *real_path = real_pathdup(ceil, 0);
if (!real_path) {
return 0;
}
free(item->string);
item->string = real_path;
return 1;
}
}

enum discovery_result {
GIT_DIR_NONE = 0,
GIT_DIR_EXPLICIT,
GIT_DIR_DISCOVERED,
GIT_DIR_BARE,

GIT_DIR_HIT_CEILING = -1,
GIT_DIR_HIT_MOUNT_POINT = -2,
GIT_DIR_INVALID_GITFILE = -3
};














static enum discovery_result setup_git_directory_gently_1(struct strbuf *dir,
struct strbuf *gitdir,
int die_on_error)
{
const char *env_ceiling_dirs = getenv(CEILING_DIRECTORIES_ENVIRONMENT);
struct string_list ceiling_dirs = STRING_LIST_INIT_DUP;
const char *gitdirenv;
int ceil_offset = -1, min_offset = offset_1st_component(dir->buf);
dev_t current_device = 0;
int one_filesystem = 1;






gitdirenv = getenv(GIT_DIR_ENVIRONMENT);
if (gitdirenv) {
strbuf_addstr(gitdir, gitdirenv);
return GIT_DIR_EXPLICIT;
}

if (env_ceiling_dirs) {
int empty_entry_found = 0;

string_list_split(&ceiling_dirs, env_ceiling_dirs, PATH_SEP, -1);
filter_string_list(&ceiling_dirs, 0,
canonicalize_ceiling_entry, &empty_entry_found);
ceil_offset = longest_ancestor_length(dir->buf, &ceiling_dirs);
string_list_clear(&ceiling_dirs, 0);
}

if (ceil_offset < 0)
ceil_offset = min_offset - 2;

if (min_offset && min_offset == dir->len &&
!is_dir_sep(dir->buf[min_offset - 1])) {
strbuf_addch(dir, '/');
min_offset++;
}












one_filesystem = !git_env_bool("GIT_DISCOVERY_ACROSS_FILESYSTEM", 0);
if (one_filesystem)
current_device = get_device_or_die(dir->buf, NULL, 0);
for (;;) {
int offset = dir->len, error_code = 0;

if (offset > min_offset)
strbuf_addch(dir, '/');
strbuf_addstr(dir, DEFAULT_GIT_DIR_ENVIRONMENT);
gitdirenv = read_gitfile_gently(dir->buf, die_on_error ?
NULL : &error_code);
if (!gitdirenv) {
if (die_on_error ||
error_code == READ_GITFILE_ERR_NOT_A_FILE) {

if (is_git_directory(dir->buf))
gitdirenv = DEFAULT_GIT_DIR_ENVIRONMENT;
} else if (error_code != READ_GITFILE_ERR_STAT_FAILED)
return GIT_DIR_INVALID_GITFILE;
}
strbuf_setlen(dir, offset);
if (gitdirenv) {
strbuf_addstr(gitdir, gitdirenv);
return GIT_DIR_DISCOVERED;
}

if (is_git_directory(dir->buf)) {
strbuf_addstr(gitdir, ".");
return GIT_DIR_BARE;
}

if (offset <= min_offset)
return GIT_DIR_HIT_CEILING;

while (--offset > ceil_offset && !is_dir_sep(dir->buf[offset]))
; 
if (offset <= ceil_offset)
return GIT_DIR_HIT_CEILING;

strbuf_setlen(dir, offset > min_offset ? offset : min_offset);
if (one_filesystem &&
current_device != get_device_or_die(dir->buf, NULL, offset))
return GIT_DIR_HIT_MOUNT_POINT;
}
}

int discover_git_directory(struct strbuf *commondir,
struct strbuf *gitdir)
{
struct strbuf dir = STRBUF_INIT, err = STRBUF_INIT;
size_t gitdir_offset = gitdir->len, cwd_len;
size_t commondir_offset = commondir->len;
struct repository_format candidate = REPOSITORY_FORMAT_INIT;

if (strbuf_getcwd(&dir))
return -1;

cwd_len = dir.len;
if (setup_git_directory_gently_1(&dir, gitdir, 0) <= 0) {
strbuf_release(&dir);
return -1;
}





if (dir.len < cwd_len && !is_absolute_path(gitdir->buf + gitdir_offset)) {

if (!strcmp(".", gitdir->buf + gitdir_offset))
strbuf_setlen(gitdir, gitdir_offset);
else
strbuf_addch(&dir, '/');
strbuf_insert(gitdir, gitdir_offset, dir.buf, dir.len);
}

get_common_dir(commondir, gitdir->buf + gitdir_offset);

strbuf_reset(&dir);
strbuf_addf(&dir, "%s/config", commondir->buf + commondir_offset);
read_repository_format(&candidate, dir.buf);
strbuf_release(&dir);

if (verify_repository_format(&candidate, &err) < 0) {
warning("ignoring git dir '%s': %s",
gitdir->buf + gitdir_offset, err.buf);
strbuf_release(&err);
strbuf_setlen(commondir, commondir_offset);
strbuf_setlen(gitdir, gitdir_offset);
clear_repository_format(&candidate);
return -1;
}

clear_repository_format(&candidate);
return 0;
}

const char *setup_git_directory_gently(int *nongit_ok)
{
static struct strbuf cwd = STRBUF_INIT;
struct strbuf dir = STRBUF_INIT, gitdir = STRBUF_INIT;
const char *prefix = NULL;
struct repository_format repo_fmt = REPOSITORY_FORMAT_INIT;








git_config_clear();






if (nongit_ok)
*nongit_ok = 0;

if (strbuf_getcwd(&cwd))
die_errno(_("Unable to read current working directory"));
strbuf_addbuf(&dir, &cwd);

switch (setup_git_directory_gently_1(&dir, &gitdir, 1)) {
case GIT_DIR_EXPLICIT:
prefix = setup_explicit_git_dir(gitdir.buf, &cwd, &repo_fmt, nongit_ok);
break;
case GIT_DIR_DISCOVERED:
if (dir.len < cwd.len && chdir(dir.buf))
die(_("cannot change to '%s'"), dir.buf);
prefix = setup_discovered_git_dir(gitdir.buf, &cwd, dir.len,
&repo_fmt, nongit_ok);
break;
case GIT_DIR_BARE:
if (dir.len < cwd.len && chdir(dir.buf))
die(_("cannot change to '%s'"), dir.buf);
prefix = setup_bare_git_dir(&cwd, dir.len, &repo_fmt, nongit_ok);
break;
case GIT_DIR_HIT_CEILING:
if (!nongit_ok)
die(_("not a git repository (or any of the parent directories): %s"),
DEFAULT_GIT_DIR_ENVIRONMENT);
*nongit_ok = 1;
break;
case GIT_DIR_HIT_MOUNT_POINT:
if (!nongit_ok)
die(_("not a git repository (or any parent up to mount point %s)\n"
"Stopping at filesystem boundary (GIT_DISCOVERY_ACROSS_FILESYSTEM not set)."),
dir.buf);
*nongit_ok = 1;
break;
case GIT_DIR_NONE:






default:
BUG("unhandled setup_git_directory_1() result");
}











if (nongit_ok && *nongit_ok) {
startup_info->have_repository = 0;
startup_info->prefix = NULL;
setenv(GIT_PREFIX_ENVIRONMENT, "", 1);
} else {
startup_info->have_repository = 1;
startup_info->prefix = prefix;
if (prefix)
setenv(GIT_PREFIX_ENVIRONMENT, prefix, 1);
else
setenv(GIT_PREFIX_ENVIRONMENT, "", 1);
}












if (
startup_info->have_repository ||

getenv(GIT_DIR_ENVIRONMENT)) {
if (!the_repository->gitdir) {
const char *gitdir = getenv(GIT_DIR_ENVIRONMENT);
if (!gitdir)
gitdir = DEFAULT_GIT_DIR_ENVIRONMENT;
setup_git_env(gitdir);
}
if (startup_info->have_repository)
repo_set_hash_algo(the_repository, repo_fmt.hash_algo);
}

strbuf_release(&dir);
strbuf_release(&gitdir);
clear_repository_format(&repo_fmt);

return prefix;
}

int git_config_perm(const char *var, const char *value)
{
int i;
char *endptr;

if (value == NULL)
return PERM_GROUP;

if (!strcmp(value, "umask"))
return PERM_UMASK;
if (!strcmp(value, "group"))
return PERM_GROUP;
if (!strcmp(value, "all") ||
!strcmp(value, "world") ||
!strcmp(value, "everybody"))
return PERM_EVERYBODY;


i = strtol(value, &endptr, 8);


if (*endptr != 0)
return git_config_bool(var, value) ? PERM_GROUP : PERM_UMASK;





switch (i) {
case PERM_UMASK: 
return PERM_UMASK;
case OLD_PERM_GROUP: 
return PERM_GROUP;
case OLD_PERM_EVERYBODY: 
return PERM_EVERYBODY;
}



if ((i & 0600) != 0600)
die(_("problem with core.sharedRepository filemode value "
"(0%.3o).\nThe owner of files must always have "
"read and write permissions."), i);





return -(i & 0666);
}

void check_repository_format(struct repository_format *fmt)
{
struct repository_format repo_fmt = REPOSITORY_FORMAT_INIT;
if (!fmt)
fmt = &repo_fmt;
check_repository_format_gently(get_git_dir(), fmt, NULL);
startup_info->have_repository = 1;
clear_repository_format(&repo_fmt);
}







const char *setup_git_directory(void)
{
return setup_git_directory_gently(NULL);
}

const char *resolve_gitdir_gently(const char *suspect, int *return_error_code)
{
if (is_git_directory(suspect))
return suspect;
return read_gitfile_gently(suspect, return_error_code);
}


void sanitize_stdfds(void)
{
int fd = open("/dev/null", O_RDWR, 0);
while (fd != -1 && fd < 2)
fd = dup(fd);
if (fd == -1)
die_errno(_("open /dev/null or dup failed"));
if (fd > 2)
close(fd);
}

int daemonize(void)
{
#if defined(NO_POSIX_GOODIES)
errno = ENOSYS;
return -1;
#else
switch (fork()) {
case 0:
break;
case -1:
die_errno(_("fork failed"));
default:
exit(0);
}
if (setsid() == -1)
die_errno(_("setsid failed"));
close(0);
close(1);
close(2);
sanitize_stdfds();
return 0;
#endif
}
