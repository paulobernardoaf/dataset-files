#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "cache.h"
#include "config.h"
#include "diff.h"
#include "commit.h"
#include "log-tree.h"
#include "builtin.h"
#include "submodule.h"
#include "repository.h"
static struct rev_info log_tree_opt;
static int diff_tree_commit_oid(const struct object_id *oid)
{
struct commit *commit = lookup_commit_reference(the_repository, oid);
if (!commit)
return -1;
return log_tree_commit(&log_tree_opt, commit);
}
static int stdin_diff_commit(struct commit *commit, const char *p)
{
struct object_id oid;
struct commit_list **pptr = NULL;
while (isspace(*p++) && !parse_oid_hex(p, &oid, &p)) {
struct commit *parent = lookup_commit(the_repository, &oid);
if (!pptr) {
free_commit_list(commit->parents);
commit->parents = NULL;
pptr = &(commit->parents);
}
if (parent) {
pptr = &commit_list_insert(parent, pptr)->next;
}
}
return log_tree_commit(&log_tree_opt, commit);
}
static int stdin_diff_trees(struct tree *tree1, const char *p)
{
struct object_id oid;
struct tree *tree2;
if (!isspace(*p++) || parse_oid_hex(p, &oid, &p) || *p)
return error("Need exactly two trees, separated by a space");
tree2 = lookup_tree(the_repository, &oid);
if (!tree2 || parse_tree(tree2))
return -1;
printf("%s %s\n", oid_to_hex(&tree1->object.oid),
oid_to_hex(&tree2->object.oid));
diff_tree_oid(&tree1->object.oid, &tree2->object.oid,
"", &log_tree_opt.diffopt);
log_tree_diff_flush(&log_tree_opt);
return 0;
}
static int diff_tree_stdin(char *line)
{
int len = strlen(line);
struct object_id oid;
struct object *obj;
const char *p;
if (!len || line[len-1] != '\n')
return -1;
line[len-1] = 0;
if (parse_oid_hex(line, &oid, &p))
return -1;
obj = parse_object(the_repository, &oid);
if (!obj)
return -1;
if (obj->type == OBJ_COMMIT)
return stdin_diff_commit((struct commit *)obj, p);
if (obj->type == OBJ_TREE)
return stdin_diff_trees((struct tree *)obj, p);
error("Object %s is a %s, not a commit or tree",
oid_to_hex(&oid), type_name(obj->type));
return -1;
}
static const char diff_tree_usage[] =
"git diff-tree [--stdin] [-m] [-c | --cc] [-s] [-v] [--pretty] [-t] [-r] [--root] "
"[<common-diff-options>] <tree-ish> [<tree-ish>] [<path>...]\n"
" -r diff recursively\n"
" -c show combined diff for merge commits\n"
" --cc show combined diff for merge commits removing uninteresting hunks\n"
" --combined-all-paths\n"
" show name of file in all parents for combined diffs\n"
" --root include the initial commit as diff against /dev/null\n"
COMMON_DIFF_OPTIONS_HELP;
static void diff_tree_tweak_rev(struct rev_info *rev, struct setup_revision_opt *opt)
{
if (!rev->diffopt.output_format) {
if (rev->dense_combined_merges)
rev->diffopt.output_format = DIFF_FORMAT_PATCH;
else
rev->diffopt.output_format = DIFF_FORMAT_RAW;
}
}
int cmd_diff_tree(int argc, const char **argv, const char *prefix)
{
char line[1000];
struct object *tree1, *tree2;
static struct rev_info *opt = &log_tree_opt;
struct setup_revision_opt s_r_opt;
int read_stdin = 0;
if (argc == 2 && !strcmp(argv[1], "-h"))
usage(diff_tree_usage);
git_config(git_diff_basic_config, NULL); 
repo_init_revisions(the_repository, opt, prefix);
if (read_cache() < 0)
die(_("index file corrupt"));
opt->abbrev = 0;
opt->diff = 1;
opt->disable_stdin = 1;
memset(&s_r_opt, 0, sizeof(s_r_opt));
s_r_opt.tweak = diff_tree_tweak_rev;
precompose_argv(argc, argv);
argc = setup_revisions(argc, argv, opt, &s_r_opt);
while (--argc > 0) {
const char *arg = *++argv;
if (!strcmp(arg, "--stdin")) {
read_stdin = 1;
continue;
}
usage(diff_tree_usage);
}
switch (opt->pending.nr) {
case 0:
if (!read_stdin)
usage(diff_tree_usage);
break;
case 1:
tree1 = opt->pending.objects[0].item;
diff_tree_commit_oid(&tree1->oid);
break;
case 2:
tree1 = opt->pending.objects[0].item;
tree2 = opt->pending.objects[1].item;
if (tree2->flags & UNINTERESTING) {
SWAP(tree2, tree1);
}
diff_tree_oid(&tree1->oid, &tree2->oid, "", &opt->diffopt);
log_tree_diff_flush(opt);
break;
}
if (read_stdin) {
int saved_nrl = 0;
int saved_dcctc = 0;
if (opt->diffopt.detect_rename) {
if (!the_index.cache)
repo_read_index(the_repository);
opt->diffopt.setup |= DIFF_SETUP_USE_SIZE_CACHE;
}
while (fgets(line, sizeof(line), stdin)) {
struct object_id oid;
if (get_oid_hex(line, &oid)) {
fputs(line, stdout);
fflush(stdout);
}
else {
diff_tree_stdin(line);
if (saved_nrl < opt->diffopt.needed_rename_limit)
saved_nrl = opt->diffopt.needed_rename_limit;
if (opt->diffopt.degraded_cc_to_c)
saved_dcctc = 1;
}
}
opt->diffopt.degraded_cc_to_c = saved_dcctc;
opt->diffopt.needed_rename_limit = saved_nrl;
}
return diff_result_code(&opt->diffopt, 0);
}
