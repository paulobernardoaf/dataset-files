#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "cache.h"
#include "repository.h"
#include "config.h"
#include "blob.h"
#include "tree.h"
#include "commit.h"
#include "tag.h"
#include "tree-walk.h"
#include "builtin.h"
#include "parse-options.h"
#include "string-list.h"
#include "run-command.h"
#include "userdiff.h"
#include "grep.h"
#include "quote.h"
#include "dir.h"
#include "pathspec.h"
#include "submodule.h"
#include "submodule-config.h"
#include "object-store.h"
#include "packfile.h"
static char const * const grep_usage[] = {
N_("git grep [<options>] [-e] <pattern> [<rev>...] [[--] <path>...]"),
NULL
};
static int recurse_submodules;
static int num_threads;
static pthread_t *threads;
struct work_item {
struct grep_source source;
char done;
struct strbuf out;
};
#define TODO_SIZE 128
static struct work_item todo[TODO_SIZE];
static int todo_start;
static int todo_end;
static int todo_done;
static int all_work_added;
static pthread_mutex_t grep_mutex;
static inline void grep_lock(void)
{
pthread_mutex_lock(&grep_mutex);
}
static inline void grep_unlock(void)
{
pthread_mutex_unlock(&grep_mutex);
}
static pthread_cond_t cond_add;
static pthread_cond_t cond_write;
static pthread_cond_t cond_result;
static int skip_first_line;
static void add_work(struct grep_opt *opt, struct grep_source *gs)
{
if (opt->binary != GREP_BINARY_TEXT)
grep_source_load_driver(gs, opt->repo->index);
grep_lock();
while ((todo_end+1) % ARRAY_SIZE(todo) == todo_done) {
pthread_cond_wait(&cond_write, &grep_mutex);
}
todo[todo_end].source = *gs;
todo[todo_end].done = 0;
strbuf_reset(&todo[todo_end].out);
todo_end = (todo_end + 1) % ARRAY_SIZE(todo);
pthread_cond_signal(&cond_add);
grep_unlock();
}
static struct work_item *get_work(void)
{
struct work_item *ret;
grep_lock();
while (todo_start == todo_end && !all_work_added) {
pthread_cond_wait(&cond_add, &grep_mutex);
}
if (todo_start == todo_end && all_work_added) {
ret = NULL;
} else {
ret = &todo[todo_start];
todo_start = (todo_start + 1) % ARRAY_SIZE(todo);
}
grep_unlock();
return ret;
}
static void work_done(struct work_item *w)
{
int old_done;
grep_lock();
w->done = 1;
old_done = todo_done;
for(; todo[todo_done].done && todo_done != todo_start;
todo_done = (todo_done+1) % ARRAY_SIZE(todo)) {
w = &todo[todo_done];
if (w->out.len) {
const char *p = w->out.buf;
size_t len = w->out.len;
if (skip_first_line) {
while (len) {
len--;
if (*p++ == '\n')
break;
}
skip_first_line = 0;
}
write_or_die(1, p, len);
}
grep_source_clear(&w->source);
}
if (old_done != todo_done)
pthread_cond_signal(&cond_write);
if (all_work_added && todo_done == todo_end)
pthread_cond_signal(&cond_result);
grep_unlock();
}
static void *run(void *arg)
{
int hit = 0;
struct grep_opt *opt = arg;
while (1) {
struct work_item *w = get_work();
if (!w)
break;
opt->output_priv = w;
hit |= grep_source(opt, &w->source);
grep_source_clear_data(&w->source);
work_done(w);
}
free_grep_patterns(arg);
free(arg);
return (void*) (intptr_t) hit;
}
static void strbuf_out(struct grep_opt *opt, const void *buf, size_t size)
{
struct work_item *w = opt->output_priv;
strbuf_add(&w->out, buf, size);
}
static void start_threads(struct grep_opt *opt)
{
int i;
pthread_mutex_init(&grep_mutex, NULL);
pthread_mutex_init(&grep_attr_mutex, NULL);
pthread_cond_init(&cond_add, NULL);
pthread_cond_init(&cond_write, NULL);
pthread_cond_init(&cond_result, NULL);
grep_use_locks = 1;
enable_obj_read_lock();
for (i = 0; i < ARRAY_SIZE(todo); i++) {
strbuf_init(&todo[i].out, 0);
}
threads = xcalloc(num_threads, sizeof(*threads));
for (i = 0; i < num_threads; i++) {
int err;
struct grep_opt *o = grep_opt_dup(opt);
o->output = strbuf_out;
if (i)
o->debug = 0;
compile_grep_patterns(o);
err = pthread_create(&threads[i], NULL, run, o);
if (err)
die(_("grep: failed to create thread: %s"),
strerror(err));
}
}
static int wait_all(void)
{
int hit = 0;
int i;
if (!HAVE_THREADS)
BUG("Never call this function unless you have started threads");
grep_lock();
all_work_added = 1;
while (todo_done != todo_end)
pthread_cond_wait(&cond_result, &grep_mutex);
pthread_cond_broadcast(&cond_add);
grep_unlock();
for (i = 0; i < num_threads; i++) {
void *h;
pthread_join(threads[i], &h);
hit |= (int) (intptr_t) h;
}
free(threads);
pthread_mutex_destroy(&grep_mutex);
pthread_mutex_destroy(&grep_attr_mutex);
pthread_cond_destroy(&cond_add);
pthread_cond_destroy(&cond_write);
pthread_cond_destroy(&cond_result);
grep_use_locks = 0;
disable_obj_read_lock();
return hit;
}
static int grep_cmd_config(const char *var, const char *value, void *cb)
{
int st = grep_config(var, value, cb);
if (git_color_default_config(var, value, cb) < 0)
st = -1;
if (!strcmp(var, "grep.threads")) {
num_threads = git_config_int(var, value);
if (num_threads < 0)
die(_("invalid number of threads specified (%d) for %s"),
num_threads, var);
else if (!HAVE_THREADS && num_threads > 1) {
warning(_("no threads support, ignoring %s"), var);
num_threads = 1;
}
}
if (!strcmp(var, "submodule.recurse"))
recurse_submodules = git_config_bool(var, value);
return st;
}
static int grep_oid(struct grep_opt *opt, const struct object_id *oid,
const char *filename, int tree_name_len,
const char *path)
{
struct strbuf pathbuf = STRBUF_INIT;
struct grep_source gs;
if (opt->relative && opt->prefix_length) {
quote_path_relative(filename + tree_name_len, opt->prefix, &pathbuf);
strbuf_insert(&pathbuf, 0, filename, tree_name_len);
} else {
strbuf_addstr(&pathbuf, filename);
}
grep_source_init(&gs, GREP_SOURCE_OID, pathbuf.buf, path, oid);
strbuf_release(&pathbuf);
if (num_threads > 1) {
add_work(opt, &gs);
return 0;
} else {
int hit;
hit = grep_source(opt, &gs);
grep_source_clear(&gs);
return hit;
}
}
static int grep_file(struct grep_opt *opt, const char *filename)
{
struct strbuf buf = STRBUF_INIT;
struct grep_source gs;
if (opt->relative && opt->prefix_length)
quote_path_relative(filename, opt->prefix, &buf);
else
strbuf_addstr(&buf, filename);
grep_source_init(&gs, GREP_SOURCE_FILE, buf.buf, filename, filename);
strbuf_release(&buf);
if (num_threads > 1) {
add_work(opt, &gs);
return 0;
} else {
int hit;
hit = grep_source(opt, &gs);
grep_source_clear(&gs);
return hit;
}
}
static void append_path(struct grep_opt *opt, const void *data, size_t len)
{
struct string_list *path_list = opt->output_priv;
if (len == 1 && *(const char *)data == '\0')
return;
string_list_append(path_list, xstrndup(data, len));
}
static void run_pager(struct grep_opt *opt, const char *prefix)
{
struct string_list *path_list = opt->output_priv;
struct child_process child = CHILD_PROCESS_INIT;
int i, status;
for (i = 0; i < path_list->nr; i++)
argv_array_push(&child.args, path_list->items[i].string);
child.dir = prefix;
child.use_shell = 1;
status = run_command(&child);
if (status)
exit(status);
}
static int grep_cache(struct grep_opt *opt,
const struct pathspec *pathspec, int cached);
static int grep_tree(struct grep_opt *opt, const struct pathspec *pathspec,
struct tree_desc *tree, struct strbuf *base, int tn_len,
int check_attr);
static int grep_submodule(struct grep_opt *opt,
const struct pathspec *pathspec,
const struct object_id *oid,
const char *filename, const char *path, int cached)
{
struct repository subrepo;
struct repository *superproject = opt->repo;
const struct submodule *sub;
struct grep_opt subopt;
int hit;
sub = submodule_from_path(superproject, &null_oid, path);
if (!is_submodule_active(superproject, path))
return 0;
if (repo_submodule_init(&subrepo, superproject, sub))
return 0;
obj_read_lock();
repo_read_gitmodules(&subrepo, 0);
add_to_alternates_memory(subrepo.objects->odb->path);
obj_read_unlock();
memcpy(&subopt, opt, sizeof(subopt));
subopt.repo = &subrepo;
if (oid) {
struct object *object;
struct tree_desc tree;
void *data;
unsigned long size;
struct strbuf base = STRBUF_INIT;
obj_read_lock();
object = parse_object_or_die(oid, oid_to_hex(oid));
obj_read_unlock();
data = read_object_with_reference(&subrepo,
&object->oid, tree_type,
&size, NULL);
if (!data)
die(_("unable to read tree (%s)"), oid_to_hex(&object->oid));
strbuf_addstr(&base, filename);
strbuf_addch(&base, '/');
init_tree_desc(&tree, data, size);
hit = grep_tree(&subopt, pathspec, &tree, &base, base.len,
object->type == OBJ_COMMIT);
strbuf_release(&base);
free(data);
} else {
hit = grep_cache(&subopt, pathspec, cached);
}
repo_clear(&subrepo);
return hit;
}
static int grep_cache(struct grep_opt *opt,
const struct pathspec *pathspec, int cached)
{
struct repository *repo = opt->repo;
int hit = 0;
int nr;
struct strbuf name = STRBUF_INIT;
int name_base_len = 0;
if (repo->submodule_prefix) {
name_base_len = strlen(repo->submodule_prefix);
strbuf_addstr(&name, repo->submodule_prefix);
}
if (repo_read_index(repo) < 0)
die(_("index file corrupt"));
for (nr = 0; nr < repo->index->cache_nr; nr++) {
const struct cache_entry *ce = repo->index->cache[nr];
strbuf_setlen(&name, name_base_len);
strbuf_addstr(&name, ce->name);
if (S_ISREG(ce->ce_mode) &&
match_pathspec(repo->index, pathspec, name.buf, name.len, 0, NULL,
S_ISDIR(ce->ce_mode) ||
S_ISGITLINK(ce->ce_mode))) {
if (cached || (ce->ce_flags & CE_VALID) ||
ce_skip_worktree(ce)) {
if (ce_stage(ce) || ce_intent_to_add(ce))
continue;
hit |= grep_oid(opt, &ce->oid, name.buf,
0, name.buf);
} else {
hit |= grep_file(opt, name.buf);
}
} else if (recurse_submodules && S_ISGITLINK(ce->ce_mode) &&
submodule_path_match(repo->index, pathspec, name.buf, NULL)) {
hit |= grep_submodule(opt, pathspec, NULL, ce->name,
ce->name, cached);
} else {
continue;
}
if (ce_stage(ce)) {
do {
nr++;
} while (nr < repo->index->cache_nr &&
!strcmp(ce->name, repo->index->cache[nr]->name));
nr--; 
}
if (hit && opt->status_only)
break;
}
strbuf_release(&name);
return hit;
}
static int grep_tree(struct grep_opt *opt, const struct pathspec *pathspec,
struct tree_desc *tree, struct strbuf *base, int tn_len,
int check_attr)
{
struct repository *repo = opt->repo;
int hit = 0;
enum interesting match = entry_not_interesting;
struct name_entry entry;
int old_baselen = base->len;
struct strbuf name = STRBUF_INIT;
int name_base_len = 0;
if (repo->submodule_prefix) {
strbuf_addstr(&name, repo->submodule_prefix);
name_base_len = name.len;
}
while (tree_entry(tree, &entry)) {
int te_len = tree_entry_len(&entry);
if (match != all_entries_interesting) {
strbuf_addstr(&name, base->buf + tn_len);
match = tree_entry_interesting(repo->index,
&entry, &name,
0, pathspec);
strbuf_setlen(&name, name_base_len);
if (match == all_entries_not_interesting)
break;
if (match == entry_not_interesting)
continue;
}
strbuf_add(base, entry.path, te_len);
if (S_ISREG(entry.mode)) {
hit |= grep_oid(opt, &entry.oid, base->buf, tn_len,
check_attr ? base->buf + tn_len : NULL);
} else if (S_ISDIR(entry.mode)) {
enum object_type type;
struct tree_desc sub;
void *data;
unsigned long size;
data = read_object_file(&entry.oid, &type, &size);
if (!data)
die(_("unable to read tree (%s)"),
oid_to_hex(&entry.oid));
strbuf_addch(base, '/');
init_tree_desc(&sub, data, size);
hit |= grep_tree(opt, pathspec, &sub, base, tn_len,
check_attr);
free(data);
} else if (recurse_submodules && S_ISGITLINK(entry.mode)) {
hit |= grep_submodule(opt, pathspec, &entry.oid,
base->buf, base->buf + tn_len,
1); 
}
strbuf_setlen(base, old_baselen);
if (hit && opt->status_only)
break;
}
strbuf_release(&name);
return hit;
}
static int grep_object(struct grep_opt *opt, const struct pathspec *pathspec,
struct object *obj, const char *name, const char *path)
{
if (obj->type == OBJ_BLOB)
return grep_oid(opt, &obj->oid, name, 0, path);
if (obj->type == OBJ_COMMIT || obj->type == OBJ_TREE) {
struct tree_desc tree;
void *data;
unsigned long size;
struct strbuf base;
int hit, len;
data = read_object_with_reference(opt->repo,
&obj->oid, tree_type,
&size, NULL);
if (!data)
die(_("unable to read tree (%s)"), oid_to_hex(&obj->oid));
len = name ? strlen(name) : 0;
strbuf_init(&base, PATH_MAX + len + 1);
if (len) {
strbuf_add(&base, name, len);
strbuf_addch(&base, ':');
}
init_tree_desc(&tree, data, size);
hit = grep_tree(opt, pathspec, &tree, &base, base.len,
obj->type == OBJ_COMMIT);
strbuf_release(&base);
free(data);
return hit;
}
die(_("unable to grep from object of type %s"), type_name(obj->type));
}
static int grep_objects(struct grep_opt *opt, const struct pathspec *pathspec,
const struct object_array *list)
{
unsigned int i;
int hit = 0;
const unsigned int nr = list->nr;
for (i = 0; i < nr; i++) {
struct object *real_obj;
obj_read_lock();
real_obj = deref_tag(opt->repo, list->objects[i].item,
NULL, 0);
obj_read_unlock();
if (recurse_submodules) {
submodule_free(opt->repo);
obj_read_lock();
gitmodules_config_oid(&real_obj->oid);
obj_read_unlock();
}
if (grep_object(opt, pathspec, real_obj, list->objects[i].name,
list->objects[i].path)) {
hit = 1;
if (opt->status_only)
break;
}
}
return hit;
}
static int grep_directory(struct grep_opt *opt, const struct pathspec *pathspec,
int exc_std, int use_index)
{
struct dir_struct dir;
int i, hit = 0;
memset(&dir, 0, sizeof(dir));
if (!use_index)
dir.flags |= DIR_NO_GITLINKS;
if (exc_std)
setup_standard_excludes(&dir);
fill_directory(&dir, opt->repo->index, pathspec);
for (i = 0; i < dir.nr; i++) {
if (!dir_path_match(opt->repo->index, dir.entries[i], pathspec, 0, NULL))
continue;
hit |= grep_file(opt, dir.entries[i]->name);
if (hit && opt->status_only)
break;
}
return hit;
}
static int context_callback(const struct option *opt, const char *arg,
int unset)
{
struct grep_opt *grep_opt = opt->value;
int value;
const char *endp;
if (unset) {
grep_opt->pre_context = grep_opt->post_context = 0;
return 0;
}
value = strtol(arg, (char **)&endp, 10);
if (*endp) {
return error(_("switch `%c' expects a numerical value"),
opt->short_name);
}
grep_opt->pre_context = grep_opt->post_context = value;
return 0;
}
static int file_callback(const struct option *opt, const char *arg, int unset)
{
struct grep_opt *grep_opt = opt->value;
int from_stdin;
FILE *patterns;
int lno = 0;
struct strbuf sb = STRBUF_INIT;
BUG_ON_OPT_NEG(unset);
from_stdin = !strcmp(arg, "-");
patterns = from_stdin ? stdin : fopen(arg, "r");
if (!patterns)
die_errno(_("cannot open '%s'"), arg);
while (strbuf_getline(&sb, patterns) == 0) {
if (sb.len == 0)
continue;
append_grep_pat(grep_opt, sb.buf, sb.len, arg, ++lno,
GREP_PATTERN);
}
if (!from_stdin)
fclose(patterns);
strbuf_release(&sb);
return 0;
}
static int not_callback(const struct option *opt, const char *arg, int unset)
{
struct grep_opt *grep_opt = opt->value;
BUG_ON_OPT_NEG(unset);
BUG_ON_OPT_ARG(arg);
append_grep_pattern(grep_opt, "--not", "command line", 0, GREP_NOT);
return 0;
}
static int and_callback(const struct option *opt, const char *arg, int unset)
{
struct grep_opt *grep_opt = opt->value;
BUG_ON_OPT_NEG(unset);
BUG_ON_OPT_ARG(arg);
append_grep_pattern(grep_opt, "--and", "command line", 0, GREP_AND);
return 0;
}
static int open_callback(const struct option *opt, const char *arg, int unset)
{
struct grep_opt *grep_opt = opt->value;
BUG_ON_OPT_NEG(unset);
BUG_ON_OPT_ARG(arg);
append_grep_pattern(grep_opt, "(", "command line", 0, GREP_OPEN_PAREN);
return 0;
}
static int close_callback(const struct option *opt, const char *arg, int unset)
{
struct grep_opt *grep_opt = opt->value;
BUG_ON_OPT_NEG(unset);
BUG_ON_OPT_ARG(arg);
append_grep_pattern(grep_opt, ")", "command line", 0, GREP_CLOSE_PAREN);
return 0;
}
static int pattern_callback(const struct option *opt, const char *arg,
int unset)
{
struct grep_opt *grep_opt = opt->value;
BUG_ON_OPT_NEG(unset);
append_grep_pattern(grep_opt, arg, "-e option", 0, GREP_PATTERN);
return 0;
}
int cmd_grep(int argc, const char **argv, const char *prefix)
{
int hit = 0;
int cached = 0, untracked = 0, opt_exclude = -1;
int seen_dashdash = 0;
int external_grep_allowed__ignored;
const char *show_in_pager = NULL, *default_pager = "dummy";
struct grep_opt opt;
struct object_array list = OBJECT_ARRAY_INIT;
struct pathspec pathspec;
struct string_list path_list = STRING_LIST_INIT_NODUP;
int i;
int dummy;
int use_index = 1;
int pattern_type_arg = GREP_PATTERN_TYPE_UNSPECIFIED;
int allow_revs;
struct option options[] = {
OPT_BOOL(0, "cached", &cached,
N_("search in index instead of in the work tree")),
OPT_NEGBIT(0, "no-index", &use_index,
N_("find in contents not managed by git"), 1),
OPT_BOOL(0, "untracked", &untracked,
N_("search in both tracked and untracked files")),
OPT_SET_INT(0, "exclude-standard", &opt_exclude,
N_("ignore files specified via '.gitignore'"), 1),
OPT_BOOL(0, "recurse-submodules", &recurse_submodules,
N_("recursively search in each submodule")),
OPT_GROUP(""),
OPT_BOOL('v', "invert-match", &opt.invert,
N_("show non-matching lines")),
OPT_BOOL('i', "ignore-case", &opt.ignore_case,
N_("case insensitive matching")),
OPT_BOOL('w', "word-regexp", &opt.word_regexp,
N_("match patterns only at word boundaries")),
OPT_SET_INT('a', "text", &opt.binary,
N_("process binary files as text"), GREP_BINARY_TEXT),
OPT_SET_INT('I', NULL, &opt.binary,
N_("don't match patterns in binary files"),
GREP_BINARY_NOMATCH),
OPT_BOOL(0, "textconv", &opt.allow_textconv,
N_("process binary files with textconv filters")),
OPT_SET_INT('r', "recursive", &opt.max_depth,
N_("search in subdirectories (default)"), -1),
{ OPTION_INTEGER, 0, "max-depth", &opt.max_depth, N_("depth"),
N_("descend at most <depth> levels"), PARSE_OPT_NONEG,
NULL, 1 },
OPT_GROUP(""),
OPT_SET_INT('E', "extended-regexp", &pattern_type_arg,
N_("use extended POSIX regular expressions"),
GREP_PATTERN_TYPE_ERE),
OPT_SET_INT('G', "basic-regexp", &pattern_type_arg,
N_("use basic POSIX regular expressions (default)"),
GREP_PATTERN_TYPE_BRE),
OPT_SET_INT('F', "fixed-strings", &pattern_type_arg,
N_("interpret patterns as fixed strings"),
GREP_PATTERN_TYPE_FIXED),
OPT_SET_INT('P', "perl-regexp", &pattern_type_arg,
N_("use Perl-compatible regular expressions"),
GREP_PATTERN_TYPE_PCRE),
OPT_GROUP(""),
OPT_BOOL('n', "line-number", &opt.linenum, N_("show line numbers")),
OPT_BOOL(0, "column", &opt.columnnum, N_("show column number of first match")),
OPT_NEGBIT('h', NULL, &opt.pathname, N_("don't show filenames"), 1),
OPT_BIT('H', NULL, &opt.pathname, N_("show filenames"), 1),
OPT_NEGBIT(0, "full-name", &opt.relative,
N_("show filenames relative to top directory"), 1),
OPT_BOOL('l', "files-with-matches", &opt.name_only,
N_("show only filenames instead of matching lines")),
OPT_BOOL(0, "name-only", &opt.name_only,
N_("synonym for --files-with-matches")),
OPT_BOOL('L', "files-without-match",
&opt.unmatch_name_only,
N_("show only the names of files without match")),
OPT_BOOL_F('z', "null", &opt.null_following_name,
N_("print NUL after filenames"),
PARSE_OPT_NOCOMPLETE),
OPT_BOOL('o', "only-matching", &opt.only_matching,
N_("show only matching parts of a line")),
OPT_BOOL('c', "count", &opt.count,
N_("show the number of matches instead of matching lines")),
OPT__COLOR(&opt.color, N_("highlight matches")),
OPT_BOOL(0, "break", &opt.file_break,
N_("print empty line between matches from different files")),
OPT_BOOL(0, "heading", &opt.heading,
N_("show filename only once above matches from same file")),
OPT_GROUP(""),
OPT_CALLBACK('C', "context", &opt, N_("n"),
N_("show <n> context lines before and after matches"),
context_callback),
OPT_INTEGER('B', "before-context", &opt.pre_context,
N_("show <n> context lines before matches")),
OPT_INTEGER('A', "after-context", &opt.post_context,
N_("show <n> context lines after matches")),
OPT_INTEGER(0, "threads", &num_threads,
N_("use <n> worker threads")),
OPT_NUMBER_CALLBACK(&opt, N_("shortcut for -C NUM"),
context_callback),
OPT_BOOL('p', "show-function", &opt.funcname,
N_("show a line with the function name before matches")),
OPT_BOOL('W', "function-context", &opt.funcbody,
N_("show the surrounding function")),
OPT_GROUP(""),
OPT_CALLBACK('f', NULL, &opt, N_("file"),
N_("read patterns from file"), file_callback),
{ OPTION_CALLBACK, 'e', NULL, &opt, N_("pattern"),
N_("match <pattern>"), PARSE_OPT_NONEG, pattern_callback },
{ OPTION_CALLBACK, 0, "and", &opt, NULL,
N_("combine patterns specified with -e"),
PARSE_OPT_NOARG | PARSE_OPT_NONEG, and_callback },
OPT_BOOL(0, "or", &dummy, ""),
{ OPTION_CALLBACK, 0, "not", &opt, NULL, "",
PARSE_OPT_NOARG | PARSE_OPT_NONEG, not_callback },
{ OPTION_CALLBACK, '(', NULL, &opt, NULL, "",
PARSE_OPT_NOARG | PARSE_OPT_NONEG | PARSE_OPT_NODASH,
open_callback },
{ OPTION_CALLBACK, ')', NULL, &opt, NULL, "",
PARSE_OPT_NOARG | PARSE_OPT_NONEG | PARSE_OPT_NODASH,
close_callback },
OPT__QUIET(&opt.status_only,
N_("indicate hit with exit status without output")),
OPT_BOOL(0, "all-match", &opt.all_match,
N_("show only matches from files that match all patterns")),
OPT_SET_INT_F(0, "debug", &opt.debug,
N_("show parse tree for grep expression"),
1, PARSE_OPT_HIDDEN),
OPT_GROUP(""),
{ OPTION_STRING, 'O', "open-files-in-pager", &show_in_pager,
N_("pager"), N_("show matching files in the pager"),
PARSE_OPT_OPTARG | PARSE_OPT_NOCOMPLETE,
NULL, (intptr_t)default_pager },
OPT_BOOL_F(0, "ext-grep", &external_grep_allowed__ignored,
N_("allow calling of grep(1) (ignored by this build)"),
PARSE_OPT_NOCOMPLETE),
OPT_END()
};
init_grep_defaults(the_repository);
git_config(grep_cmd_config, NULL);
grep_init(&opt, the_repository, prefix);
argc = parse_options(argc, argv, prefix, options, grep_usage,
PARSE_OPT_KEEP_DASHDASH |
PARSE_OPT_STOP_AT_NON_OPTION);
grep_commit_pattern_type(pattern_type_arg, &opt);
if (use_index && !startup_info->have_repository) {
int fallback = 0;
git_config_get_bool("grep.fallbacktonoindex", &fallback);
if (fallback)
use_index = 0;
else
setup_git_directory();
}
if (!use_index)
recurse_submodules = 0;
if (argc > 0 && !opt.pattern_list && !strcmp(argv[0], "--")) {
argv++;
argc--;
}
if (argc > 0 && !opt.pattern_list) {
append_grep_pattern(&opt, argv[0], "command line", 0,
GREP_PATTERN);
argv++;
argc--;
}
if (show_in_pager == default_pager)
show_in_pager = git_pager(1);
if (show_in_pager) {
opt.color = 0;
opt.name_only = 1;
opt.null_following_name = 1;
opt.output_priv = &path_list;
opt.output = append_path;
string_list_append(&path_list, show_in_pager);
}
if (!opt.pattern_list)
die(_("no pattern given"));
if (opt.invert)
opt.only_matching = 0;
for (i = 0; i < argc; i++) {
if (!strcmp(argv[i], "--")) {
seen_dashdash = 1;
break;
}
}
allow_revs = use_index && !untracked;
for (i = 0; i < argc; i++) {
const char *arg = argv[i];
struct object_id oid;
struct object_context oc;
struct object *object;
if (!strcmp(arg, "--")) {
i++;
break;
}
if (!allow_revs) {
if (seen_dashdash)
die(_("--no-index or --untracked cannot be used with revs"));
break;
}
if (get_oid_with_context(the_repository, arg,
GET_OID_RECORD_PATH,
&oid, &oc)) {
if (seen_dashdash)
die(_("unable to resolve revision: %s"), arg);
break;
}
object = parse_object_or_die(&oid, arg);
if (!seen_dashdash)
verify_non_filename(prefix, arg);
add_object_array_with_path(object, arg, &list, oc.mode, oc.path);
free(oc.path);
}
if (!seen_dashdash) {
int j;
for (j = i; j < argc; j++)
verify_filename(prefix, argv[j], j == i && allow_revs);
}
parse_pathspec(&pathspec, 0,
PATHSPEC_PREFER_CWD |
(opt.max_depth != -1 ? PATHSPEC_MAXDEPTH_VALID : 0),
prefix, argv + i);
pathspec.max_depth = opt.max_depth;
pathspec.recursive = 1;
pathspec.recurse_submodules = !!recurse_submodules;
if (recurse_submodules && untracked)
die(_("--untracked not supported with --recurse-submodules"));
if (show_in_pager) {
if (num_threads > 1)
warning(_("invalid option combination, ignoring --threads"));
num_threads = 1;
} else if (!HAVE_THREADS && num_threads > 1) {
warning(_("no threads support, ignoring --threads"));
num_threads = 1;
} else if (num_threads < 0)
die(_("invalid number of threads specified (%d)"), num_threads);
else if (num_threads == 0)
num_threads = HAVE_THREADS ? online_cpus() : 1;
if (num_threads > 1) {
if (!HAVE_THREADS)
BUG("Somebody got num_threads calculation wrong!");
if (!(opt.name_only || opt.unmatch_name_only || opt.count)
&& (opt.pre_context || opt.post_context ||
opt.file_break || opt.funcbody))
skip_first_line = 1;
if (recurse_submodules)
repo_read_gitmodules(the_repository, 1);
if (startup_info->have_repository)
(void)get_packed_git(the_repository);
start_threads(&opt);
} else {
compile_grep_patterns(&opt);
}
if (show_in_pager && (cached || list.nr))
die(_("--open-files-in-pager only works on the worktree"));
if (show_in_pager && opt.pattern_list && !opt.pattern_list->next) {
const char *pager = path_list.items[0].string;
int len = strlen(pager);
if (len > 4 && is_dir_sep(pager[len - 5]))
pager += len - 4;
if (opt.ignore_case && !strcmp("less", pager))
string_list_append(&path_list, "-I");
if (!strcmp("less", pager) || !strcmp("vi", pager)) {
struct strbuf buf = STRBUF_INIT;
strbuf_addf(&buf, "+/%s%s",
strcmp("less", pager) ? "" : "*",
opt.pattern_list->pattern);
string_list_append(&path_list,
strbuf_detach(&buf, NULL));
}
}
if (!show_in_pager && !opt.status_only)
setup_pager();
if (!use_index && (untracked || cached))
die(_("--cached or --untracked cannot be used with --no-index"));
if (!use_index || untracked) {
int use_exclude = (opt_exclude < 0) ? use_index : !!opt_exclude;
hit = grep_directory(&opt, &pathspec, use_exclude, use_index);
} else if (0 <= opt_exclude) {
die(_("--[no-]exclude-standard cannot be used for tracked contents"));
} else if (!list.nr) {
if (!cached)
setup_work_tree();
hit = grep_cache(&opt, &pathspec, cached);
} else {
if (cached)
die(_("both --cached and trees are given"));
hit = grep_objects(&opt, &pathspec, &list);
}
if (num_threads > 1)
hit |= wait_all();
if (hit && show_in_pager)
run_pager(&opt, prefix);
clear_pathspec(&pathspec);
free_grep_patterns(&opt);
grep_destroy();
return !hit;
}
