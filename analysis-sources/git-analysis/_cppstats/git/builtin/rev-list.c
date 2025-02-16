#include "cache.h"
#include "config.h"
#include "commit.h"
#include "diff.h"
#include "revision.h"
#include "list-objects.h"
#include "list-objects-filter.h"
#include "list-objects-filter-options.h"
#include "object.h"
#include "object-store.h"
#include "pack.h"
#include "pack-bitmap.h"
#include "builtin.h"
#include "log-tree.h"
#include "graph.h"
#include "bisect.h"
#include "progress.h"
#include "reflog-walk.h"
#include "oidset.h"
#include "packfile.h"
static const char rev_list_usage[] =
"git rev-list [OPTION] <commit-id>... [ -- paths... ]\n"
" limiting output:\n"
" --max-count=<n>\n"
" --max-age=<epoch>\n"
" --min-age=<epoch>\n"
" --sparse\n"
" --no-merges\n"
" --min-parents=<n>\n"
" --no-min-parents\n"
" --max-parents=<n>\n"
" --no-max-parents\n"
" --remove-empty\n"
" --all\n"
" --branches\n"
" --tags\n"
" --remotes\n"
" --stdin\n"
" --quiet\n"
" ordering output:\n"
" --topo-order\n"
" --date-order\n"
" --reverse\n"
" formatting output:\n"
" --parents\n"
" --children\n"
" --objects | --objects-edge\n"
" --unpacked\n"
" --header | --pretty\n"
" --[no-]object-names\n"
" --abbrev=<n> | --no-abbrev\n"
" --abbrev-commit\n"
" --left-right\n"
" --count\n"
" special purpose:\n"
" --bisect\n"
" --bisect-vars\n"
" --bisect-all"
;
static struct progress *progress;
static unsigned progress_counter;
static struct list_objects_filter_options filter_options;
static struct oidset omitted_objects;
static int arg_print_omitted; 
static struct oidset missing_objects;
enum missing_action {
MA_ERROR = 0, 
MA_ALLOW_ANY, 
MA_PRINT, 
MA_ALLOW_PROMISOR, 
};
static enum missing_action arg_missing_action;
static int arg_show_object_names = 1;
#define DEFAULT_OIDSET_SIZE (16*1024)
static void finish_commit(struct commit *commit);
static void show_commit(struct commit *commit, void *data)
{
struct rev_list_info *info = data;
struct rev_info *revs = info->revs;
display_progress(progress, ++progress_counter);
if (info->flags & REV_LIST_QUIET) {
finish_commit(commit);
return;
}
graph_show_commit(revs->graph);
if (revs->count) {
if (commit->object.flags & PATCHSAME)
revs->count_same++;
else if (commit->object.flags & SYMMETRIC_LEFT)
revs->count_left++;
else
revs->count_right++;
finish_commit(commit);
return;
}
if (info->show_timestamp)
printf("%"PRItime" ", commit->date);
if (info->header_prefix)
fputs(info->header_prefix, stdout);
if (!revs->graph)
fputs(get_revision_mark(revs, commit), stdout);
if (revs->abbrev_commit && revs->abbrev)
fputs(find_unique_abbrev(&commit->object.oid, revs->abbrev),
stdout);
else
fputs(oid_to_hex(&commit->object.oid), stdout);
if (revs->print_parents) {
struct commit_list *parents = commit->parents;
while (parents) {
printf(" %s", oid_to_hex(&parents->item->object.oid));
parents = parents->next;
}
}
if (revs->children.name) {
struct commit_list *children;
children = lookup_decoration(&revs->children, &commit->object);
while (children) {
printf(" %s", oid_to_hex(&children->item->object.oid));
children = children->next;
}
}
show_decorations(revs, commit);
if (revs->commit_format == CMIT_FMT_ONELINE)
putchar(' ');
else
putchar('\n');
if (revs->verbose_header) {
struct strbuf buf = STRBUF_INIT;
struct pretty_print_context ctx = {0};
ctx.abbrev = revs->abbrev;
ctx.date_mode = revs->date_mode;
ctx.date_mode_explicit = revs->date_mode_explicit;
ctx.fmt = revs->commit_format;
ctx.output_encoding = get_log_output_encoding();
ctx.color = revs->diffopt.use_color;
pretty_print_commit(&ctx, commit, &buf);
if (buf.len) {
if (revs->commit_format != CMIT_FMT_ONELINE)
graph_show_oneline(revs->graph);
graph_show_commit_msg(revs->graph, stdout, &buf);
if (buf.len && buf.buf[buf.len - 1] == '\n')
graph_show_padding(revs->graph);
putchar(info->hdr_termination);
} else {
if (graph_show_remainder(revs->graph))
putchar('\n');
if (revs->commit_format == CMIT_FMT_ONELINE)
putchar('\n');
}
strbuf_release(&buf);
} else {
if (graph_show_remainder(revs->graph))
putchar('\n');
}
maybe_flush_or_die(stdout, "stdout");
finish_commit(commit);
}
static void finish_commit(struct commit *commit)
{
if (commit->parents) {
free_commit_list(commit->parents);
commit->parents = NULL;
}
free_commit_buffer(the_repository->parsed_objects,
commit);
}
static inline void finish_object__ma(struct object *obj)
{
switch (arg_missing_action) {
case MA_ERROR:
die("missing %s object '%s'",
type_name(obj->type), oid_to_hex(&obj->oid));
return;
case MA_ALLOW_ANY:
return;
case MA_PRINT:
oidset_insert(&missing_objects, &obj->oid);
return;
case MA_ALLOW_PROMISOR:
if (is_promisor_object(&obj->oid))
return;
die("unexpected missing %s object '%s'",
type_name(obj->type), oid_to_hex(&obj->oid));
return;
default:
BUG("unhandled missing_action");
return;
}
}
static int finish_object(struct object *obj, const char *name, void *cb_data)
{
struct rev_list_info *info = cb_data;
if (oid_object_info_extended(the_repository, &obj->oid, NULL, 0) < 0) {
finish_object__ma(obj);
return 1;
}
if (info->revs->verify_objects && !obj->parsed && obj->type != OBJ_COMMIT)
parse_object(the_repository, &obj->oid);
return 0;
}
static void show_object(struct object *obj, const char *name, void *cb_data)
{
struct rev_list_info *info = cb_data;
struct rev_info *revs = info->revs;
if (finish_object(obj, name, cb_data))
return;
display_progress(progress, ++progress_counter);
if (info->flags & REV_LIST_QUIET)
return;
if (revs->count) {
revs->count_right++;
return;
}
if (arg_show_object_names)
show_object_with_name(stdout, obj, name);
else
printf("%s\n", oid_to_hex(&obj->oid));
}
static void show_edge(struct commit *commit)
{
printf("-%s\n", oid_to_hex(&commit->object.oid));
}
static void print_var_str(const char *var, const char *val)
{
printf("%s='%s'\n", var, val);
}
static void print_var_int(const char *var, int val)
{
printf("%s=%d\n", var, val);
}
static int show_bisect_vars(struct rev_list_info *info, int reaches, int all)
{
int cnt, flags = info->flags;
char hex[GIT_MAX_HEXSZ + 1] = "";
struct commit_list *tried;
struct rev_info *revs = info->revs;
if (!revs->commits)
return 1;
revs->commits = filter_skipped(revs->commits, &tried,
flags & BISECT_SHOW_ALL,
NULL, NULL);
cnt = all - reaches;
if (cnt < reaches)
cnt = reaches;
if (revs->commits)
oid_to_hex_r(hex, &revs->commits->item->object.oid);
if (flags & BISECT_SHOW_ALL) {
traverse_commit_list(revs, show_commit, show_object, info);
printf("------\n");
}
print_var_str("bisect_rev", hex);
print_var_int("bisect_nr", cnt - 1);
print_var_int("bisect_good", all - reaches - 1);
print_var_int("bisect_bad", reaches - 1);
print_var_int("bisect_all", all);
print_var_int("bisect_steps", estimate_bisect_steps(all));
return 0;
}
static int show_object_fast(
const struct object_id *oid,
enum object_type type,
int exclude,
uint32_t name_hash,
struct packed_git *found_pack,
off_t found_offset)
{
fprintf(stdout, "%s\n", oid_to_hex(oid));
return 1;
}
static inline int parse_missing_action_value(const char *value)
{
if (!strcmp(value, "error")) {
arg_missing_action = MA_ERROR;
return 1;
}
if (!strcmp(value, "allow-any")) {
arg_missing_action = MA_ALLOW_ANY;
fetch_if_missing = 0;
return 1;
}
if (!strcmp(value, "print")) {
arg_missing_action = MA_PRINT;
fetch_if_missing = 0;
return 1;
}
if (!strcmp(value, "allow-promisor")) {
arg_missing_action = MA_ALLOW_PROMISOR;
fetch_if_missing = 0;
return 1;
}
return 0;
}
static int try_bitmap_count(struct rev_info *revs,
struct list_objects_filter_options *filter)
{
uint32_t commit_count = 0,
tag_count = 0,
tree_count = 0,
blob_count = 0;
int max_count;
struct bitmap_index *bitmap_git;
if (!revs->count)
return -1;
if (revs->left_right || revs->cherry_mark)
return -1;
if (revs->max_count >= 0 &&
(revs->tag_objects || revs->tree_objects || revs->blob_objects))
return -1;
max_count = revs->max_count;
bitmap_git = prepare_bitmap_walk(revs, filter);
if (!bitmap_git)
return -1;
count_bitmap_commit_list(bitmap_git, &commit_count,
revs->tree_objects ? &tree_count : NULL,
revs->blob_objects ? &blob_count : NULL,
revs->tag_objects ? &tag_count : NULL);
if (max_count >= 0 && max_count < commit_count)
commit_count = max_count;
printf("%d\n", commit_count + tree_count + blob_count + tag_count);
free_bitmap_index(bitmap_git);
return 0;
}
static int try_bitmap_traversal(struct rev_info *revs,
struct list_objects_filter_options *filter)
{
struct bitmap_index *bitmap_git;
if (revs->max_count >= 0)
return -1;
bitmap_git = prepare_bitmap_walk(revs, filter);
if (!bitmap_git)
return -1;
traverse_bitmap_commit_list(bitmap_git, revs, &show_object_fast);
free_bitmap_index(bitmap_git);
return 0;
}
int cmd_rev_list(int argc, const char **argv, const char *prefix)
{
struct rev_info revs;
struct rev_list_info info;
struct setup_revision_opt s_r_opt = {
.allow_exclude_promisor_objects = 1,
};
int i;
int bisect_list = 0;
int bisect_show_vars = 0;
int bisect_find_all = 0;
int use_bitmap_index = 0;
const char *show_progress = NULL;
if (argc == 2 && !strcmp(argv[1], "-h"))
usage(rev_list_usage);
git_config(git_default_config, NULL);
repo_init_revisions(the_repository, &revs, prefix);
revs.abbrev = DEFAULT_ABBREV;
revs.commit_format = CMIT_FMT_UNSPECIFIED;
for (i = 1; i < argc; i++) {
const char *arg = argv[i];
if (!strcmp(arg, "--exclude-promisor-objects")) {
fetch_if_missing = 0;
revs.exclude_promisor_objects = 1;
break;
}
}
for (i = 1; i < argc; i++) {
const char *arg = argv[i];
if (skip_prefix(arg, "--missing=", &arg)) {
if (revs.exclude_promisor_objects)
die(_("cannot combine --exclude-promisor-objects and --missing"));
if (parse_missing_action_value(arg))
break;
}
}
if (arg_missing_action)
revs.do_not_die_on_missing_tree = 1;
argc = setup_revisions(argc, argv, &revs, &s_r_opt);
memset(&info, 0, sizeof(info));
info.revs = &revs;
if (revs.bisect)
bisect_list = 1;
if (revs.diffopt.flags.quick)
info.flags |= REV_LIST_QUIET;
for (i = 1 ; i < argc; i++) {
const char *arg = argv[i];
if (!strcmp(arg, "--header")) {
revs.verbose_header = 1;
continue;
}
if (!strcmp(arg, "--timestamp")) {
info.show_timestamp = 1;
continue;
}
if (!strcmp(arg, "--bisect")) {
bisect_list = 1;
continue;
}
if (!strcmp(arg, "--bisect-all")) {
bisect_list = 1;
bisect_find_all = 1;
info.flags |= BISECT_SHOW_ALL;
revs.show_decorations = 1;
continue;
}
if (!strcmp(arg, "--bisect-vars")) {
bisect_list = 1;
bisect_show_vars = 1;
continue;
}
if (!strcmp(arg, "--use-bitmap-index")) {
use_bitmap_index = 1;
continue;
}
if (!strcmp(arg, "--test-bitmap")) {
test_bitmap_walk(&revs);
return 0;
}
if (skip_prefix(arg, "--progress=", &arg)) {
show_progress = arg;
continue;
}
if (skip_prefix(arg, ("--" CL_ARG__FILTER "="), &arg)) {
parse_list_objects_filter(&filter_options, arg);
if (filter_options.choice && !revs.blob_objects)
die(_("object filtering requires --objects"));
continue;
}
if (!strcmp(arg, ("--no-" CL_ARG__FILTER))) {
list_objects_filter_set_no_filter(&filter_options);
continue;
}
if (!strcmp(arg, "--filter-print-omitted")) {
arg_print_omitted = 1;
continue;
}
if (!strcmp(arg, "--exclude-promisor-objects"))
continue; 
if (skip_prefix(arg, "--missing=", &arg))
continue; 
if (!strcmp(arg, ("--no-object-names"))) {
arg_show_object_names = 0;
continue;
}
if (!strcmp(arg, ("--object-names"))) {
arg_show_object_names = 1;
continue;
}
usage(rev_list_usage);
}
if (revs.commit_format != CMIT_FMT_UNSPECIFIED) {
info.hdr_termination = '\n';
if (revs.commit_format == CMIT_FMT_ONELINE)
info.header_prefix = "";
else
info.header_prefix = "commit ";
}
else if (revs.verbose_header)
revs.commit_format = CMIT_FMT_RAW;
if ((!revs.commits && reflog_walk_empty(revs.reflog_info) &&
(!(revs.tag_objects || revs.tree_objects || revs.blob_objects) &&
!revs.pending.nr) &&
!revs.rev_input_given && !revs.read_from_stdin) ||
revs.diff)
usage(rev_list_usage);
if (revs.show_notes)
die(_("rev-list does not support display of notes"));
if (revs.count &&
(revs.tag_objects || revs.tree_objects || revs.blob_objects) &&
(revs.left_right || revs.cherry_mark))
die(_("marked counting is incompatible with --objects"));
save_commit_buffer = (revs.verbose_header ||
revs.grep_filter.pattern_list ||
revs.grep_filter.header_list);
if (bisect_list)
revs.limited = 1;
if (show_progress)
progress = start_delayed_progress(show_progress, 0);
if (use_bitmap_index) {
if (!try_bitmap_count(&revs, &filter_options))
return 0;
if (!try_bitmap_traversal(&revs, &filter_options))
return 0;
}
if (prepare_revision_walk(&revs))
die("revision walk setup failed");
if (revs.tree_objects)
mark_edges_uninteresting(&revs, show_edge, 0);
if (bisect_list) {
int reaches, all;
find_bisection(&revs.commits, &reaches, &all, bisect_find_all);
if (bisect_show_vars)
return show_bisect_vars(&info, reaches, all);
}
if (arg_print_omitted)
oidset_init(&omitted_objects, DEFAULT_OIDSET_SIZE);
if (arg_missing_action == MA_PRINT)
oidset_init(&missing_objects, DEFAULT_OIDSET_SIZE);
traverse_commit_list_filtered(
&filter_options, &revs, show_commit, show_object, &info,
(arg_print_omitted ? &omitted_objects : NULL));
if (arg_print_omitted) {
struct oidset_iter iter;
struct object_id *oid;
oidset_iter_init(&omitted_objects, &iter);
while ((oid = oidset_iter_next(&iter)))
printf("~%s\n", oid_to_hex(oid));
oidset_clear(&omitted_objects);
}
if (arg_missing_action == MA_PRINT) {
struct oidset_iter iter;
struct object_id *oid;
oidset_iter_init(&missing_objects, &iter);
while ((oid = oidset_iter_next(&iter)))
printf("?%s\n", oid_to_hex(oid));
oidset_clear(&missing_objects);
}
stop_progress(&progress);
if (revs.count) {
if (revs.left_right && revs.cherry_mark)
printf("%d\t%d\t%d\n", revs.count_left, revs.count_right, revs.count_same);
else if (revs.left_right)
printf("%d\t%d\n", revs.count_left, revs.count_right);
else if (revs.cherry_mark)
printf("%d\t%d\n", revs.count_left + revs.count_right, revs.count_same);
else
printf("%d\n", revs.count_left + revs.count_right);
}
return 0;
}
