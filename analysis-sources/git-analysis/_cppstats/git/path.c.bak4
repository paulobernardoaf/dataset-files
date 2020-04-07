


#include "cache.h"
#include "repository.h"
#include "strbuf.h"
#include "string-list.h"
#include "dir.h"
#include "worktree.h"
#include "submodule-config.h"
#include "path.h"
#include "packfile.h"
#include "object-store.h"
#include "lockfile.h"

static int get_st_mode_bits(const char *path, int *mode)
{
struct stat st;
if (lstat(path, &st) < 0)
return -1;
*mode = st.st_mode;
return 0;
}

static char bad_path[] = "/bad-path/";

static struct strbuf *get_pathname(void)
{
static struct strbuf pathname_array[4] = {
STRBUF_INIT, STRBUF_INIT, STRBUF_INIT, STRBUF_INIT
};
static int index;
struct strbuf *sb = &pathname_array[index];
index = (index + 1) % ARRAY_SIZE(pathname_array);
strbuf_reset(sb);
return sb;
}

static const char *cleanup_path(const char *path)
{

if (skip_prefix(path, "./", &path)) {
while (*path == '/')
path++;
}
return path;
}

static void strbuf_cleanup_path(struct strbuf *sb)
{
const char *path = cleanup_path(sb->buf);
if (path > sb->buf)
strbuf_remove(sb, 0, path - sb->buf);
}

char *mksnpath(char *buf, size_t n, const char *fmt, ...)
{
va_list args;
unsigned len;

va_start(args, fmt);
len = vsnprintf(buf, n, fmt, args);
va_end(args);
if (len >= n) {
strlcpy(buf, bad_path, n);
return buf;
}
return (char *)cleanup_path(buf);
}

static int dir_prefix(const char *buf, const char *dir)
{
int len = strlen(dir);
return !strncmp(buf, dir, len) &&
(is_dir_sep(buf[len]) || buf[len] == '\0');
}


static int is_dir_file(const char *buf, const char *dir, const char *file)
{
int len = strlen(dir);
if (strncmp(buf, dir, len) || !is_dir_sep(buf[len]))
return 0;
while (is_dir_sep(buf[len]))
len++;
return !strcmp(buf + len, file);
}

static void replace_dir(struct strbuf *buf, int len, const char *newdir)
{
int newlen = strlen(newdir);
int need_sep = (buf->buf[len] && !is_dir_sep(buf->buf[len])) &&
!is_dir_sep(newdir[newlen - 1]);
if (need_sep)
len--; 
strbuf_splice(buf, 0, len, newdir, newlen);
if (need_sep)
buf->buf[newlen] = '/';
}

struct common_dir {

unsigned ignore_garbage:1;
unsigned is_dir:1;

unsigned is_common:1;
const char *path;
};

static struct common_dir common_list[] = {
{ 0, 1, 1, "branches" },
{ 0, 1, 1, "common" },
{ 0, 1, 1, "hooks" },
{ 0, 1, 1, "info" },
{ 0, 0, 0, "info/sparse-checkout" },
{ 1, 1, 1, "logs" },
{ 1, 0, 0, "logs/HEAD" },
{ 0, 1, 0, "logs/refs/bisect" },
{ 0, 1, 0, "logs/refs/rewritten" },
{ 0, 1, 0, "logs/refs/worktree" },
{ 0, 1, 1, "lost-found" },
{ 0, 1, 1, "objects" },
{ 0, 1, 1, "refs" },
{ 0, 1, 0, "refs/bisect" },
{ 0, 1, 0, "refs/rewritten" },
{ 0, 1, 0, "refs/worktree" },
{ 0, 1, 1, "remotes" },
{ 0, 1, 1, "worktrees" },
{ 0, 1, 1, "rr-cache" },
{ 0, 1, 1, "svn" },
{ 0, 0, 1, "config" },
{ 1, 0, 1, "gc.pid" },
{ 0, 0, 1, "packed-refs" },
{ 0, 0, 1, "shallow" },
{ 0, 0, 0, NULL }
};





















struct trie {
struct trie *children[256];
int len;
char *contents;
void *value;
};

static struct trie *make_trie_node(const char *key, void *value)
{
struct trie *new_node = xcalloc(1, sizeof(*new_node));
new_node->len = strlen(key);
if (new_node->len) {
new_node->contents = xmalloc(new_node->len);
memcpy(new_node->contents, key, new_node->len);
}
new_node->value = value;
return new_node;
}





static void *add_to_trie(struct trie *root, const char *key, void *value)
{
struct trie *child;
void *old;
int i;

if (!*key) {

old = root->value;
root->value = value;
return old;
}

for (i = 0; i < root->len; i++) {
if (root->contents[i] == key[i])
continue;





child = xmalloc(sizeof(*child));
memcpy(child->children, root->children, sizeof(root->children));

child->len = root->len - i - 1;
if (child->len) {
child->contents = xstrndup(root->contents + i + 1,
child->len);
}
child->value = root->value;
root->value = NULL;
root->len = i;

memset(root->children, 0, sizeof(root->children));
root->children[(unsigned char)root->contents[i]] = child;


root->children[(unsigned char)key[i]] =
make_trie_node(key + i + 1, value);
return NULL;
}


if (key[i]) {
child = root->children[(unsigned char)key[root->len]];
if (child) {
return add_to_trie(child, key + root->len + 1, value);
} else {
child = make_trie_node(key + root->len + 1, value);
root->children[(unsigned char)key[root->len]] = child;
return NULL;
}
}

old = root->value;
root->value = value;
return old;
}

typedef int (*match_fn)(const char *unmatched, void *value, void *baton);



































static int trie_find(struct trie *root, const char *key, match_fn fn,
void *baton)
{
int i;
int result;
struct trie *child;

if (!*key) {

if (root->value && !root->len)
return fn(key, root->value, baton);
else
return -1;
}

for (i = 0; i < root->len; i++) {

if (key[i] == '/' && key[i+1] == '/') {
key++;
continue;
}
if (root->contents[i] != key[i])
return -1;
}


key += i;
if (!*key) {

if (root->value)
return fn(key, root->value, baton);
else
return -1;
}


while (key[0] == '/' && key[1] == '/')
key++;

child = root->children[(unsigned char)*key];
if (child)
result = trie_find(child, key + 1, fn, baton);
else
result = -1;

if (result >= 0 || (*key != '/' && *key != 0))
return result;
if (root->value)
return fn(key, root->value, baton);
else
return -1;
}

static struct trie common_trie;
static int common_trie_done_setup;

static void init_common_trie(void)
{
struct common_dir *p;

if (common_trie_done_setup)
return;

for (p = common_list; p->path; p++)
add_to_trie(&common_trie, p->path, p);

common_trie_done_setup = 1;
}





static int check_common(const char *unmatched, void *value, void *baton)
{
struct common_dir *dir = value;

if (dir->is_dir && (unmatched[0] == 0 || unmatched[0] == '/'))
return dir->is_common;

if (!dir->is_dir && unmatched[0] == 0)
return dir->is_common;

return 0;
}

static void update_common_dir(struct strbuf *buf, int git_dir_len,
const char *common_dir)
{
char *base = buf->buf + git_dir_len;
int has_lock_suffix = strbuf_strip_suffix(buf, LOCK_SUFFIX);

init_common_trie();
if (trie_find(&common_trie, base, check_common, NULL) > 0)
replace_dir(buf, git_dir_len, common_dir);

if (has_lock_suffix)
strbuf_addstr(buf, LOCK_SUFFIX);
}

void report_linked_checkout_garbage(void)
{
struct strbuf sb = STRBUF_INIT;
const struct common_dir *p;
int len;

if (!the_repository->different_commondir)
return;
strbuf_addf(&sb, "%s/", get_git_dir());
len = sb.len;
for (p = common_list; p->path; p++) {
const char *path = p->path;
if (p->ignore_garbage)
continue;
strbuf_setlen(&sb, len);
strbuf_addstr(&sb, path);
if (file_exists(sb.buf))
report_garbage(PACKDIR_FILE_GARBAGE, sb.buf);
}
strbuf_release(&sb);
}

static void adjust_git_path(const struct repository *repo,
struct strbuf *buf, int git_dir_len)
{
const char *base = buf->buf + git_dir_len;
if (is_dir_file(base, "info", "grafts"))
strbuf_splice(buf, 0, buf->len,
repo->graft_file, strlen(repo->graft_file));
else if (!strcmp(base, "index"))
strbuf_splice(buf, 0, buf->len,
repo->index_file, strlen(repo->index_file));
else if (dir_prefix(base, "objects"))
replace_dir(buf, git_dir_len + 7, repo->objects->odb->path);
else if (git_hooks_path && dir_prefix(base, "hooks"))
replace_dir(buf, git_dir_len + 5, git_hooks_path);
else if (repo->different_commondir)
update_common_dir(buf, git_dir_len, repo->commondir);
}

static void strbuf_worktree_gitdir(struct strbuf *buf,
const struct repository *repo,
const struct worktree *wt)
{
if (!wt)
strbuf_addstr(buf, repo->gitdir);
else if (!wt->id)
strbuf_addstr(buf, repo->commondir);
else
strbuf_git_common_path(buf, repo, "worktrees/%s", wt->id);
}

static void do_git_path(const struct repository *repo,
const struct worktree *wt, struct strbuf *buf,
const char *fmt, va_list args)
{
int gitdir_len;
strbuf_worktree_gitdir(buf, repo, wt);
if (buf->len && !is_dir_sep(buf->buf[buf->len - 1]))
strbuf_addch(buf, '/');
gitdir_len = buf->len;
strbuf_vaddf(buf, fmt, args);
if (!wt)
adjust_git_path(repo, buf, gitdir_len);
strbuf_cleanup_path(buf);
}

char *repo_git_path(const struct repository *repo,
const char *fmt, ...)
{
struct strbuf path = STRBUF_INIT;
va_list args;
va_start(args, fmt);
do_git_path(repo, NULL, &path, fmt, args);
va_end(args);
return strbuf_detach(&path, NULL);
}

void strbuf_repo_git_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
do_git_path(repo, NULL, sb, fmt, args);
va_end(args);
}

char *git_path_buf(struct strbuf *buf, const char *fmt, ...)
{
va_list args;
strbuf_reset(buf);
va_start(args, fmt);
do_git_path(the_repository, NULL, buf, fmt, args);
va_end(args);
return buf->buf;
}

void strbuf_git_path(struct strbuf *sb, const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
do_git_path(the_repository, NULL, sb, fmt, args);
va_end(args);
}

const char *git_path(const char *fmt, ...)
{
struct strbuf *pathname = get_pathname();
va_list args;
va_start(args, fmt);
do_git_path(the_repository, NULL, pathname, fmt, args);
va_end(args);
return pathname->buf;
}

char *git_pathdup(const char *fmt, ...)
{
struct strbuf path = STRBUF_INIT;
va_list args;
va_start(args, fmt);
do_git_path(the_repository, NULL, &path, fmt, args);
va_end(args);
return strbuf_detach(&path, NULL);
}

char *mkpathdup(const char *fmt, ...)
{
struct strbuf sb = STRBUF_INIT;
va_list args;
va_start(args, fmt);
strbuf_vaddf(&sb, fmt, args);
va_end(args);
strbuf_cleanup_path(&sb);
return strbuf_detach(&sb, NULL);
}

const char *mkpath(const char *fmt, ...)
{
va_list args;
struct strbuf *pathname = get_pathname();
va_start(args, fmt);
strbuf_vaddf(pathname, fmt, args);
va_end(args);
return cleanup_path(pathname->buf);
}

const char *worktree_git_path(const struct worktree *wt, const char *fmt, ...)
{
struct strbuf *pathname = get_pathname();
va_list args;
va_start(args, fmt);
do_git_path(the_repository, wt, pathname, fmt, args);
va_end(args);
return pathname->buf;
}

static void do_worktree_path(const struct repository *repo,
struct strbuf *buf,
const char *fmt, va_list args)
{
strbuf_addstr(buf, repo->worktree);
if(buf->len && !is_dir_sep(buf->buf[buf->len - 1]))
strbuf_addch(buf, '/');

strbuf_vaddf(buf, fmt, args);
strbuf_cleanup_path(buf);
}

char *repo_worktree_path(const struct repository *repo, const char *fmt, ...)
{
struct strbuf path = STRBUF_INIT;
va_list args;

if (!repo->worktree)
return NULL;

va_start(args, fmt);
do_worktree_path(repo, &path, fmt, args);
va_end(args);

return strbuf_detach(&path, NULL);
}

void strbuf_repo_worktree_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
{
va_list args;

if (!repo->worktree)
return;

va_start(args, fmt);
do_worktree_path(repo, sb, fmt, args);
va_end(args);
}


static int do_submodule_path(struct strbuf *buf, const char *path,
const char *fmt, va_list args)
{
struct strbuf git_submodule_common_dir = STRBUF_INIT;
struct strbuf git_submodule_dir = STRBUF_INIT;
int ret;

ret = submodule_to_gitdir(&git_submodule_dir, path);
if (ret)
goto cleanup;

strbuf_complete(&git_submodule_dir, '/');
strbuf_addbuf(buf, &git_submodule_dir);
strbuf_vaddf(buf, fmt, args);

if (get_common_dir_noenv(&git_submodule_common_dir, git_submodule_dir.buf))
update_common_dir(buf, git_submodule_dir.len, git_submodule_common_dir.buf);

strbuf_cleanup_path(buf);

cleanup:
strbuf_release(&git_submodule_dir);
strbuf_release(&git_submodule_common_dir);
return ret;
}

char *git_pathdup_submodule(const char *path, const char *fmt, ...)
{
int err;
va_list args;
struct strbuf buf = STRBUF_INIT;
va_start(args, fmt);
err = do_submodule_path(&buf, path, fmt, args);
va_end(args);
if (err) {
strbuf_release(&buf);
return NULL;
}
return strbuf_detach(&buf, NULL);
}

int strbuf_git_path_submodule(struct strbuf *buf, const char *path,
const char *fmt, ...)
{
int err;
va_list args;
va_start(args, fmt);
err = do_submodule_path(buf, path, fmt, args);
va_end(args);

return err;
}

static void do_git_common_path(const struct repository *repo,
struct strbuf *buf,
const char *fmt,
va_list args)
{
strbuf_addstr(buf, repo->commondir);
if (buf->len && !is_dir_sep(buf->buf[buf->len - 1]))
strbuf_addch(buf, '/');
strbuf_vaddf(buf, fmt, args);
strbuf_cleanup_path(buf);
}

const char *git_common_path(const char *fmt, ...)
{
struct strbuf *pathname = get_pathname();
va_list args;
va_start(args, fmt);
do_git_common_path(the_repository, pathname, fmt, args);
va_end(args);
return pathname->buf;
}

void strbuf_git_common_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
{
va_list args;
va_start(args, fmt);
do_git_common_path(repo, sb, fmt, args);
va_end(args);
}

int validate_headref(const char *path)
{
struct stat st;
char buffer[256];
const char *refname;
struct object_id oid;
int fd;
ssize_t len;

if (lstat(path, &st) < 0)
return -1;


if (S_ISLNK(st.st_mode)) {
len = readlink(path, buffer, sizeof(buffer)-1);
if (len >= 5 && !memcmp("refs/", buffer, 5))
return 0;
return -1;
}




fd = open(path, O_RDONLY);
if (fd < 0)
return -1;
len = read_in_full(fd, buffer, sizeof(buffer)-1);
close(fd);

if (len < 0)
return -1;
buffer[len] = '\0';




if (skip_prefix(buffer, "ref:", &refname)) {
while (isspace(*refname))
refname++;
if (starts_with(refname, "refs/"))
return 0;
}




if (!get_oid_hex(buffer, &oid))
return 0;

return -1;
}

static struct passwd *getpw_str(const char *username, size_t len)
{
struct passwd *pw;
char *username_z = xmemdupz(username, len);
pw = getpwnam(username_z);
free(username_z);
return pw;
}








char *expand_user_path(const char *path, int real_home)
{
struct strbuf user_path = STRBUF_INIT;
const char *to_copy = path;

if (path == NULL)
goto return_null;
if (path[0] == '~') {
const char *first_slash = strchrnul(path, '/');
const char *username = path + 1;
size_t username_len = first_slash - username;
if (username_len == 0) {
const char *home = getenv("HOME");
if (!home)
goto return_null;
if (real_home)
strbuf_add_real_path(&user_path, home);
else
strbuf_addstr(&user_path, home);
#if defined(GIT_WINDOWS_NATIVE)
convert_slashes(user_path.buf);
#endif
} else {
struct passwd *pw = getpw_str(username, username_len);
if (!pw)
goto return_null;
strbuf_addstr(&user_path, pw->pw_dir);
}
to_copy = first_slash;
}
strbuf_addstr(&user_path, to_copy);
return strbuf_detach(&user_path, NULL);
return_null:
strbuf_release(&user_path);
return NULL;
}




















const char *enter_repo(const char *path, int strict)
{
static struct strbuf validated_path = STRBUF_INIT;
static struct strbuf used_path = STRBUF_INIT;

if (!path)
return NULL;

if (!strict) {
static const char *suffix[] = {
"/.git", "", ".git/.git", ".git", NULL,
};
const char *gitfile;
int len = strlen(path);
int i;
while ((1 < len) && (path[len-1] == '/'))
len--;





if (PATH_MAX <= len)
return NULL;

strbuf_reset(&used_path);
strbuf_reset(&validated_path);
strbuf_add(&used_path, path, len);
strbuf_add(&validated_path, path, len);

if (used_path.buf[0] == '~') {
char *newpath = expand_user_path(used_path.buf, 0);
if (!newpath)
return NULL;
strbuf_attach(&used_path, newpath, strlen(newpath),
strlen(newpath));
}
for (i = 0; suffix[i]; i++) {
struct stat st;
size_t baselen = used_path.len;
strbuf_addstr(&used_path, suffix[i]);
if (!stat(used_path.buf, &st) &&
(S_ISREG(st.st_mode) ||
(S_ISDIR(st.st_mode) && is_git_directory(used_path.buf)))) {
strbuf_addstr(&validated_path, suffix[i]);
break;
}
strbuf_setlen(&used_path, baselen);
}
if (!suffix[i])
return NULL;
gitfile = read_gitfile(used_path.buf);
if (gitfile) {
strbuf_reset(&used_path);
strbuf_addstr(&used_path, gitfile);
}
if (chdir(used_path.buf))
return NULL;
path = validated_path.buf;
}
else {
const char *gitfile = read_gitfile(path);
if (gitfile)
path = gitfile;
if (chdir(path))
return NULL;
}

if (is_git_directory(".")) {
set_git_dir(".", 0);
check_repository_format(NULL);
return path;
}

return NULL;
}

static int calc_shared_perm(int mode)
{
int tweak;

if (get_shared_repository() < 0)
tweak = -get_shared_repository();
else
tweak = get_shared_repository();

if (!(mode & S_IWUSR))
tweak &= ~0222;
if (mode & S_IXUSR)

tweak |= (tweak & 0444) >> 2;
if (get_shared_repository() < 0)
mode = (mode & ~0777) | tweak;
else
mode |= tweak;

return mode;
}


int adjust_shared_perm(const char *path)
{
int old_mode, new_mode;

if (!get_shared_repository())
return 0;
if (get_st_mode_bits(path, &old_mode) < 0)
return -1;

new_mode = calc_shared_perm(old_mode);
if (S_ISDIR(old_mode)) {

new_mode |= (new_mode & 0444) >> 2;
new_mode |= FORCE_DIR_SET_GID;
}

if (((old_mode ^ new_mode) & ~S_IFMT) &&
chmod(path, (new_mode & ~S_IFMT)) < 0)
return -2;
return 0;
}

void safe_create_dir(const char *dir, int share)
{
if (mkdir(dir, 0777) < 0) {
if (errno != EEXIST) {
perror(dir);
exit(1);
}
}
else if (share && adjust_shared_perm(dir))
die(_("Could not make %s writable by group"), dir);
}

static int have_same_root(const char *path1, const char *path2)
{
int is_abs1, is_abs2;

is_abs1 = is_absolute_path(path1);
is_abs2 = is_absolute_path(path2);
return (is_abs1 && is_abs2 && tolower(path1[0]) == tolower(path2[0])) ||
(!is_abs1 && !is_abs2);
}







const char *relative_path(const char *in, const char *prefix,
struct strbuf *sb)
{
int in_len = in ? strlen(in) : 0;
int prefix_len = prefix ? strlen(prefix) : 0;
int in_off = 0;
int prefix_off = 0;
int i = 0, j = 0;

if (!in_len)
return "./";
else if (!prefix_len)
return in;

if (have_same_root(in, prefix))

i = j = has_dos_drive_prefix(in);
else {
return in;
}

while (i < prefix_len && j < in_len && prefix[i] == in[j]) {
if (is_dir_sep(prefix[i])) {
while (is_dir_sep(prefix[i]))
i++;
while (is_dir_sep(in[j]))
j++;
prefix_off = i;
in_off = j;
} else {
i++;
j++;
}
}

if (

i >= prefix_len &&




prefix_off < prefix_len) {
if (j >= in_len) {

in_off = in_len;
} else if (is_dir_sep(in[j])) {

while (is_dir_sep(in[j]))
j++;
in_off = j;
} else {

i = prefix_off;
}
} else if (

j >= in_len &&

in_off < in_len) {
if (is_dir_sep(prefix[i])) {

while (is_dir_sep(prefix[i]))
i++;
in_off = in_len;
}
}
in += in_off;
in_len -= in_off;

if (i >= prefix_len) {
if (!in_len)
return "./";
else
return in;
}

strbuf_reset(sb);
strbuf_grow(sb, in_len);

while (i < prefix_len) {
if (is_dir_sep(prefix[i])) {
strbuf_addstr(sb, "../");
while (is_dir_sep(prefix[i]))
i++;
continue;
}
i++;
}
if (!is_dir_sep(prefix[prefix_len - 1]))
strbuf_addstr(sb, "../");

strbuf_addstr(sb, in);

return sb->buf;
}








const char *remove_leading_path(const char *in, const char *prefix)
{
static struct strbuf buf = STRBUF_INIT;
int i = 0, j = 0;

if (!prefix || !prefix[0])
return in;
while (prefix[i]) {
if (is_dir_sep(prefix[i])) {
if (!is_dir_sep(in[j]))
return in;
while (is_dir_sep(prefix[i]))
i++;
while (is_dir_sep(in[j]))
j++;
continue;
} else if (in[j] != prefix[i]) {
return in;
}
i++;
j++;
}
if (

in[j] &&

!is_dir_sep(prefix[i-1]) && !is_dir_sep(in[j])
)
return in;
while (is_dir_sep(in[j]))
j++;

strbuf_reset(&buf);
if (!in[j])
strbuf_addstr(&buf, ".");
else
strbuf_addstr(&buf, in + j);
return buf.buf;
}




























int normalize_path_copy_len(char *dst, const char *src, int *prefix_len)
{
char *dst0;
const char *end;




end = src + offset_1st_component(src);
while (src < end) {
char c = *src++;
if (is_dir_sep(c))
c = '/';
*dst++ = c;
}
dst0 = dst;

while (is_dir_sep(*src))
src++;

for (;;) {
char c = *src;









if (c == '.') {
if (!src[1]) {

src++;
} else if (is_dir_sep(src[1])) {

src += 2;
while (is_dir_sep(*src))
src++;
continue;
} else if (src[1] == '.') {
if (!src[2]) {

src += 2;
goto up_one;
} else if (is_dir_sep(src[2])) {

src += 3;
while (is_dir_sep(*src))
src++;
goto up_one;
}
}
}


while ((c = *src++) != '\0' && !is_dir_sep(c))
*dst++ = c;
if (is_dir_sep(c)) {
*dst++ = '/';
while (is_dir_sep(c))
c = *src++;
src--;
} else if (!c)
break;
continue;

up_one:




dst--; 
if (dst <= dst0)
return -1;

while (dst0 < dst && dst[-1] != '/')
dst--;
if (prefix_len && *prefix_len > dst - dst0)
*prefix_len = dst - dst0;
}
*dst = '\0';
return 0;
}

int normalize_path_copy(char *dst, const char *src)
{
return normalize_path_copy_len(dst, src, NULL);
}















int longest_ancestor_length(const char *path, struct string_list *prefixes)
{
int i, max_len = -1;

if (!strcmp(path, "/"))
return -1;

for (i = 0; i < prefixes->nr; i++) {
const char *ceil = prefixes->items[i].string;
int len = strlen(ceil);

if (len == 1 && ceil[0] == '/')
len = 0; 
else if (!strncmp(path, ceil, len) && path[len] == '/')
; 
else
continue; 

if (len > max_len)
max_len = len;
}

return max_len;
}


static inline int chomp_trailing_dir_sep(const char *path, int len)
{
while (len && is_dir_sep(path[len - 1]))
len--;
return len;
}






static ssize_t stripped_path_suffix_offset(const char *path, const char *suffix)
{
int path_len = strlen(path), suffix_len = strlen(suffix);

while (suffix_len) {
if (!path_len)
return -1;

if (is_dir_sep(path[path_len - 1])) {
if (!is_dir_sep(suffix[suffix_len - 1]))
return -1;
path_len = chomp_trailing_dir_sep(path, path_len);
suffix_len = chomp_trailing_dir_sep(suffix, suffix_len);
}
else if (path[--path_len] != suffix[--suffix_len])
return -1;
}

if (path_len && !is_dir_sep(path[path_len - 1]))
return -1;
return chomp_trailing_dir_sep(path, path_len);
}





int ends_with_path_components(const char *path, const char *components)
{
return stripped_path_suffix_offset(path, components) != -1;
}






char *strip_path_suffix(const char *path, const char *suffix)
{
ssize_t offset = stripped_path_suffix_offset(path, suffix);

return offset == -1 ? NULL : xstrndup(path, offset);
}

int daemon_avoid_alias(const char *p)
{
int sl, ndot;










if (!p || (*p != '/' && *p != '~'))
return -1;
sl = 1; ndot = 0;
p++;

while (1) {
char ch = *p++;
if (sl) {
if (ch == '.')
ndot++;
else if (ch == '/') {
if (ndot < 3)

return -1;
ndot = 0;
}
else if (ch == 0) {
if (0 < ndot && ndot < 3)

return -1;
return 0;
}
else
sl = ndot = 0;
}
else if (ch == 0)
return 0;
else if (ch == '/') {
sl = 1;
ndot = 0;
}
}
}






































int is_ntfs_dotgit(const char *name)
{
char c;







c = *(name++);
if (c == '.') {

if (((c = *(name++)) != 'g' && c != 'G') ||
((c = *(name++)) != 'i' && c != 'I') ||
((c = *(name++)) != 't' && c != 'T'))
return 0;
} else if (c == 'g' || c == 'G') {

if (((c = *(name++)) != 'i' && c != 'I') ||
((c = *(name++)) != 't' && c != 'T') ||
*(name++) != '~' ||
*(name++) != '1')
return 0;
} else
return 0;

for (;;) {
c = *(name++);
if (!c || c == '\\' || c == '/' || c == ':')
return 1;
if (c != '.' && c != ' ')
return 0;
}
}

static int is_ntfs_dot_generic(const char *name,
const char *dotgit_name,
size_t len,
const char *dotgit_ntfs_shortname_prefix)
{
int saw_tilde;
size_t i;

if ((name[0] == '.' && !strncasecmp(name + 1, dotgit_name, len))) {
i = len + 1;
only_spaces_and_periods:
for (;;) {
char c = name[i++];
if (!c || c == ':')
return 1;
if (c != ' ' && c != '.')
return 0;
}
}





if (!strncasecmp(name, dotgit_name, 6) && name[6] == '~' &&
name[7] >= '1' && name[7] <= '4') {
i = 8;
goto only_spaces_and_periods;
}





for (i = 0, saw_tilde = 0; i < 8; i++)
if (name[i] == '\0')
return 0;
else if (saw_tilde) {
if (name[i] < '0' || name[i] > '9')
return 0;
} else if (name[i] == '~') {
if (name[++i] < '1' || name[i] > '9')
return 0;
saw_tilde = 1;
} else if (i >= 6)
return 0;
else if (name[i] & 0x80) {




return 0;
} else if (tolower(name[i]) != dotgit_ntfs_shortname_prefix[i])
return 0;

goto only_spaces_and_periods;
}





static inline int is_ntfs_dot_str(const char *name, const char *dotgit_name,
const char *dotgit_ntfs_shortname_prefix)
{
return is_ntfs_dot_generic(name, dotgit_name, strlen(dotgit_name),
dotgit_ntfs_shortname_prefix);
}

int is_ntfs_dotgitmodules(const char *name)
{
return is_ntfs_dot_str(name, "gitmodules", "gi7eba");
}

int is_ntfs_dotgitignore(const char *name)
{
return is_ntfs_dot_str(name, "gitignore", "gi250a");
}

int is_ntfs_dotgitattributes(const char *name)
{
return is_ntfs_dot_str(name, "gitattributes", "gi7d29");
}

int looks_like_command_line_option(const char *str)
{
return str && str[0] == '-';
}

char *xdg_config_home(const char *filename)
{
const char *home, *config_home;

assert(filename);
config_home = getenv("XDG_CONFIG_HOME");
if (config_home && *config_home)
return mkpathdup("%s/git/%s", config_home, filename);

home = getenv("HOME");
if (home)
return mkpathdup("%s/.config/git/%s", home, filename);
return NULL;
}

char *xdg_cache_home(const char *filename)
{
const char *home, *cache_home;

assert(filename);
cache_home = getenv("XDG_CACHE_HOME");
if (cache_home && *cache_home)
return mkpathdup("%s/git/%s", cache_home, filename);

home = getenv("HOME");
if (home)
return mkpathdup("%s/.cache/git/%s", home, filename);
return NULL;
}

REPO_GIT_PATH_FUNC(cherry_pick_head, "CHERRY_PICK_HEAD")
REPO_GIT_PATH_FUNC(revert_head, "REVERT_HEAD")
REPO_GIT_PATH_FUNC(squash_msg, "SQUASH_MSG")
REPO_GIT_PATH_FUNC(merge_msg, "MERGE_MSG")
REPO_GIT_PATH_FUNC(merge_rr, "MERGE_RR")
REPO_GIT_PATH_FUNC(merge_mode, "MERGE_MODE")
REPO_GIT_PATH_FUNC(merge_head, "MERGE_HEAD")
REPO_GIT_PATH_FUNC(fetch_head, "FETCH_HEAD")
REPO_GIT_PATH_FUNC(shallow, "shallow")
