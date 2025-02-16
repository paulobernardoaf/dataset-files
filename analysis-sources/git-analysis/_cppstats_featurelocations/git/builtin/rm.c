




#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "builtin.h"
#include "config.h"
#include "lockfile.h"
#include "dir.h"
#include "cache-tree.h"
#include "tree-walk.h"
#include "parse-options.h"
#include "string-list.h"
#include "submodule.h"
#include "pathspec.h"

static const char * const builtin_rm_usage[] = {
N_("git rm [<options>] [--] <file>..."),
NULL
};

static struct {
int nr, alloc;
struct {
const char *name;
char is_submodule;
} *entry;
} list;

static int get_ours_cache_pos(const char *path, int pos)
{
int i = -pos - 1;

while ((i < active_nr) && !strcmp(active_cache[i]->name, path)) {
if (ce_stage(active_cache[i]) == 2)
return i;
i++;
}
return -1;
}

static void print_error_files(struct string_list *files_list,
const char *main_msg,
const char *hints_msg,
int *errs)
{
if (files_list->nr) {
int i;
struct strbuf err_msg = STRBUF_INIT;

strbuf_addstr(&err_msg, main_msg);
for (i = 0; i < files_list->nr; i++)
strbuf_addf(&err_msg,
"\n %s",
files_list->items[i].string);
if (advice_rm_hints)
strbuf_addstr(&err_msg, hints_msg);
*errs = error("%s", err_msg.buf);
strbuf_release(&err_msg);
}
}

static void submodules_absorb_gitdir_if_needed(void)
{
int i;
for (i = 0; i < list.nr; i++) {
const char *name = list.entry[i].name;
int pos;
const struct cache_entry *ce;

pos = cache_name_pos(name, strlen(name));
if (pos < 0) {
pos = get_ours_cache_pos(name, pos);
if (pos < 0)
continue;
}
ce = active_cache[pos];

if (!S_ISGITLINK(ce->ce_mode) ||
!file_exists(ce->name) ||
is_empty_dir(name))
continue;

if (!submodule_uses_gitfile(name))
absorb_git_dir_into_superproject(name,
ABSORB_GITDIR_RECURSE_SUBMODULES);
}
}

static int check_local_mod(struct object_id *head, int index_only)
{







int i, no_head;
int errs = 0;
struct string_list files_staged = STRING_LIST_INIT_NODUP;
struct string_list files_cached = STRING_LIST_INIT_NODUP;
struct string_list files_local = STRING_LIST_INIT_NODUP;

no_head = is_null_oid(head);
for (i = 0; i < list.nr; i++) {
struct stat st;
int pos;
const struct cache_entry *ce;
const char *name = list.entry[i].name;
struct object_id oid;
unsigned short mode;
int local_changes = 0;
int staged_changes = 0;

pos = cache_name_pos(name, strlen(name));
if (pos < 0) {




pos = get_ours_cache_pos(name, pos);
if (pos < 0)
continue;

if (!S_ISGITLINK(active_cache[pos]->ce_mode) ||
is_empty_dir(name))
continue;
}
ce = active_cache[pos];

if (lstat(ce->name, &st) < 0) {
if (!is_missing_file_error(errno))
warning_errno(_("failed to stat '%s'"), ce->name);

continue;
}
else if (S_ISDIR(st.st_mode)) {





if (!S_ISGITLINK(ce->ce_mode))
continue;
}




















if (ce_match_stat(ce, &st, 0) ||
(S_ISGITLINK(ce->ce_mode) &&
bad_to_remove_submodule(ce->name,
SUBMODULE_REMOVAL_DIE_ON_ERROR |
SUBMODULE_REMOVAL_IGNORE_IGNORED_UNTRACKED)))
local_changes = 1;







if (no_head
|| get_tree_entry(the_repository, head, name, &oid, &mode)
|| ce->ce_mode != create_ce_mode(mode)
|| !oideq(&ce->oid, &oid))
staged_changes = 1;









if (local_changes && staged_changes) {
if (!index_only || !ce_intent_to_add(ce))
string_list_append(&files_staged, name);
}
else if (!index_only) {
if (staged_changes)
string_list_append(&files_cached, name);
if (local_changes)
string_list_append(&files_local, name);
}
}
print_error_files(&files_staged,
Q_("the following file has staged content different "
"from both the\nfile and the HEAD:",
"the following files have staged content different"
" from both the\nfile and the HEAD:",
files_staged.nr),
_("\n(use -f to force removal)"),
&errs);
string_list_clear(&files_staged, 0);
print_error_files(&files_cached,
Q_("the following file has changes "
"staged in the index:",
"the following files have changes "
"staged in the index:", files_cached.nr),
_("\n(use --cached to keep the file,"
" or -f to force removal)"),
&errs);
string_list_clear(&files_cached, 0);

print_error_files(&files_local,
Q_("the following file has local modifications:",
"the following files have local modifications:",
files_local.nr),
_("\n(use --cached to keep the file,"
" or -f to force removal)"),
&errs);
string_list_clear(&files_local, 0);

return errs;
}

static int show_only = 0, force = 0, index_only = 0, recursive = 0, quiet = 0;
static int ignore_unmatch = 0, pathspec_file_nul;
static char *pathspec_from_file;

static struct option builtin_rm_options[] = {
OPT__DRY_RUN(&show_only, N_("dry run")),
OPT__QUIET(&quiet, N_("do not list removed files")),
OPT_BOOL( 0 , "cached", &index_only, N_("only remove from the index")),
OPT__FORCE(&force, N_("override the up-to-date check"), PARSE_OPT_NOCOMPLETE),
OPT_BOOL('r', NULL, &recursive, N_("allow recursive removal")),
OPT_BOOL( 0 , "ignore-unmatch", &ignore_unmatch,
N_("exit with a zero status even if nothing matched")),
OPT_PATHSPEC_FROM_FILE(&pathspec_from_file),
OPT_PATHSPEC_FILE_NUL(&pathspec_file_nul),
OPT_END(),
};

int cmd_rm(int argc, const char **argv, const char *prefix)
{
struct lock_file lock_file = LOCK_INIT;
int i;
struct pathspec pathspec;
char *seen;

git_config(git_default_config, NULL);

argc = parse_options(argc, argv, prefix, builtin_rm_options,
builtin_rm_usage, 0);

parse_pathspec(&pathspec, 0,
PATHSPEC_PREFER_CWD,
prefix, argv);

if (pathspec_from_file) {
if (pathspec.nr)
die(_("--pathspec-from-file is incompatible with pathspec arguments"));

parse_pathspec_file(&pathspec, 0,
PATHSPEC_PREFER_CWD,
prefix, pathspec_from_file, pathspec_file_nul);
} else if (pathspec_file_nul) {
die(_("--pathspec-file-nul requires --pathspec-from-file"));
}

if (!pathspec.nr)
die(_("No pathspec was given. Which files should I remove?"));

if (!index_only)
setup_work_tree();

hold_locked_index(&lock_file, LOCK_DIE_ON_ERROR);

if (read_cache() < 0)
die(_("index file corrupt"));

refresh_index(&the_index, REFRESH_QUIET|REFRESH_UNMERGED, &pathspec, NULL, NULL);

seen = xcalloc(pathspec.nr, 1);

for (i = 0; i < active_nr; i++) {
const struct cache_entry *ce = active_cache[i];
if (!ce_path_match(&the_index, ce, &pathspec, seen))
continue;
ALLOC_GROW(list.entry, list.nr + 1, list.alloc);
list.entry[list.nr].name = xstrdup(ce->name);
list.entry[list.nr].is_submodule = S_ISGITLINK(ce->ce_mode);
if (list.entry[list.nr++].is_submodule &&
!is_staging_gitmodules_ok(&the_index))
die(_("please stage your changes to .gitmodules or stash them to proceed"));
}

if (pathspec.nr) {
const char *original;
int seen_any = 0;
for (i = 0; i < pathspec.nr; i++) {
original = pathspec.items[i].original;
if (!seen[i]) {
if (!ignore_unmatch) {
die(_("pathspec '%s' did not match any files"),
original);
}
}
else {
seen_any = 1;
}
if (!recursive && seen[i] == MATCHED_RECURSIVELY)
die(_("not removing '%s' recursively without -r"),
*original ? original : ".");
}

if (!seen_any)
exit(0);
}

if (!index_only)
submodules_absorb_gitdir_if_needed();











if (!force) {
struct object_id oid;
if (get_oid("HEAD", &oid))
oidclr(&oid);
if (check_local_mod(&oid, index_only))
exit(1);
}





for (i = 0; i < list.nr; i++) {
const char *path = list.entry[i].name;
if (!quiet)
printf("rm '%s'\n", path);

if (remove_file_from_cache(path))
die(_("git rm: unable to remove %s"), path);
}

if (show_only)
return 0;









if (!index_only) {
int removed = 0, gitmodules_modified = 0;
struct strbuf buf = STRBUF_INIT;
for (i = 0; i < list.nr; i++) {
const char *path = list.entry[i].name;
if (list.entry[i].is_submodule) {
strbuf_reset(&buf);
strbuf_addstr(&buf, path);
if (remove_dir_recursively(&buf, 0))
die(_("could not remove '%s'"), path);

removed = 1;
if (!remove_path_from_gitmodules(path))
gitmodules_modified = 1;
continue;
}
if (!remove_path(path)) {
removed = 1;
continue;
}
if (!removed)
die_errno("git rm: '%s'", path);
}
strbuf_release(&buf);
if (gitmodules_modified)
stage_updated_gitmodules(&the_index);
}

if (write_locked_index(&the_index, &lock_file,
COMMIT_LOCK | SKIP_IF_UNCHANGED))
die(_("Unable to write new index file"));

return 0;
}
