#include "builtin.h"
#include "config.h"
#include "dir.h"
#include "lockfile.h"
#include "parse-options.h"
#include "repository.h"
#include "commit-graph.h"
#include "object-store.h"
static char const * const builtin_commit_graph_usage[] = {
N_("git commit-graph verify [--object-dir <objdir>] [--shallow] [--[no-]progress]"),
N_("git commit-graph write [--object-dir <objdir>] [--append|--split] [--reachable|--stdin-packs|--stdin-commits] [--[no-]progress] <split options>"),
NULL
};
static const char * const builtin_commit_graph_verify_usage[] = {
N_("git commit-graph verify [--object-dir <objdir>] [--shallow] [--[no-]progress]"),
NULL
};
static const char * const builtin_commit_graph_write_usage[] = {
N_("git commit-graph write [--object-dir <objdir>] [--append|--split] [--reachable|--stdin-packs|--stdin-commits] [--[no-]progress] <split options>"),
NULL
};
static struct opts_commit_graph {
const char *obj_dir;
int reachable;
int stdin_packs;
int stdin_commits;
int append;
int split;
int shallow;
int progress;
} opts;
static struct object_directory *find_odb(struct repository *r,
const char *obj_dir)
{
struct object_directory *odb;
char *obj_dir_real = real_pathdup(obj_dir, 1);
struct strbuf odb_path_real = STRBUF_INIT;
prepare_alt_odb(r);
for (odb = r->objects->odb; odb; odb = odb->next) {
strbuf_realpath(&odb_path_real, odb->path, 1);
if (!strcmp(obj_dir_real, odb_path_real.buf))
break;
}
free(obj_dir_real);
strbuf_release(&odb_path_real);
if (!odb)
die(_("could not find object directory matching %s"), obj_dir);
return odb;
}
static int graph_verify(int argc, const char **argv)
{
struct commit_graph *graph = NULL;
struct object_directory *odb = NULL;
char *graph_name;
int open_ok;
int fd;
struct stat st;
int flags = 0;
static struct option builtin_commit_graph_verify_options[] = {
OPT_STRING(0, "object-dir", &opts.obj_dir,
N_("dir"),
N_("The object directory to store the graph")),
OPT_BOOL(0, "shallow", &opts.shallow,
N_("if the commit-graph is split, only verify the tip file")),
OPT_BOOL(0, "progress", &opts.progress, N_("force progress reporting")),
OPT_END(),
};
trace2_cmd_mode("verify");
opts.progress = isatty(2);
argc = parse_options(argc, argv, NULL,
builtin_commit_graph_verify_options,
builtin_commit_graph_verify_usage, 0);
if (!opts.obj_dir)
opts.obj_dir = get_object_directory();
if (opts.shallow)
flags |= COMMIT_GRAPH_VERIFY_SHALLOW;
if (opts.progress)
flags |= COMMIT_GRAPH_WRITE_PROGRESS;
odb = find_odb(the_repository, opts.obj_dir);
graph_name = get_commit_graph_filename(odb);
open_ok = open_commit_graph(graph_name, &fd, &st);
if (!open_ok && errno != ENOENT)
die_errno(_("Could not open commit-graph '%s'"), graph_name);
FREE_AND_NULL(graph_name);
if (open_ok)
graph = load_commit_graph_one_fd_st(fd, &st, odb);
else
graph = read_commit_graph_one(the_repository, odb);
if (!graph)
return !!open_ok;
UNLEAK(graph);
return verify_commit_graph(the_repository, graph, flags);
}
extern int read_replace_refs;
static struct split_commit_graph_opts split_opts;
static int graph_write(int argc, const char **argv)
{
struct string_list *pack_indexes = NULL;
struct string_list *commit_hex = NULL;
struct object_directory *odb = NULL;
struct string_list lines;
int result = 0;
enum commit_graph_write_flags flags = 0;
static struct option builtin_commit_graph_write_options[] = {
OPT_STRING(0, "object-dir", &opts.obj_dir,
N_("dir"),
N_("The object directory to store the graph")),
OPT_BOOL(0, "reachable", &opts.reachable,
N_("start walk at all refs")),
OPT_BOOL(0, "stdin-packs", &opts.stdin_packs,
N_("scan pack-indexes listed by stdin for commits")),
OPT_BOOL(0, "stdin-commits", &opts.stdin_commits,
N_("start walk at commits listed by stdin")),
OPT_BOOL(0, "append", &opts.append,
N_("include all commits already in the commit-graph file")),
OPT_BOOL(0, "progress", &opts.progress, N_("force progress reporting")),
OPT_BOOL(0, "split", &opts.split,
N_("allow writing an incremental commit-graph file")),
OPT_INTEGER(0, "max-commits", &split_opts.max_commits,
N_("maximum number of commits in a non-base split commit-graph")),
OPT_INTEGER(0, "size-multiple", &split_opts.size_multiple,
N_("maximum ratio between two levels of a split commit-graph")),
OPT_EXPIRY_DATE(0, "expire-time", &split_opts.expire_time,
N_("maximum number of commits in a non-base split commit-graph")),
OPT_END(),
};
opts.progress = isatty(2);
split_opts.size_multiple = 2;
split_opts.max_commits = 0;
split_opts.expire_time = 0;
trace2_cmd_mode("write");
argc = parse_options(argc, argv, NULL,
builtin_commit_graph_write_options,
builtin_commit_graph_write_usage, 0);
if (opts.reachable + opts.stdin_packs + opts.stdin_commits > 1)
die(_("use at most one of --reachable, --stdin-commits, or --stdin-packs"));
if (!opts.obj_dir)
opts.obj_dir = get_object_directory();
if (opts.append)
flags |= COMMIT_GRAPH_WRITE_APPEND;
if (opts.split)
flags |= COMMIT_GRAPH_WRITE_SPLIT;
if (opts.progress)
flags |= COMMIT_GRAPH_WRITE_PROGRESS;
read_replace_refs = 0;
odb = find_odb(the_repository, opts.obj_dir);
if (opts.reachable) {
if (write_commit_graph_reachable(odb, flags, &split_opts))
return 1;
return 0;
}
string_list_init(&lines, 0);
if (opts.stdin_packs || opts.stdin_commits) {
struct strbuf buf = STRBUF_INIT;
while (strbuf_getline(&buf, stdin) != EOF)
string_list_append(&lines, strbuf_detach(&buf, NULL));
if (opts.stdin_packs)
pack_indexes = &lines;
if (opts.stdin_commits) {
commit_hex = &lines;
flags |= COMMIT_GRAPH_WRITE_CHECK_OIDS;
}
UNLEAK(buf);
}
if (write_commit_graph(odb,
pack_indexes,
commit_hex,
flags,
&split_opts))
result = 1;
UNLEAK(lines);
return result;
}
int cmd_commit_graph(int argc, const char **argv, const char *prefix)
{
static struct option builtin_commit_graph_options[] = {
OPT_STRING(0, "object-dir", &opts.obj_dir,
N_("dir"),
N_("The object directory to store the graph")),
OPT_END(),
};
if (argc == 2 && !strcmp(argv[1], "-h"))
usage_with_options(builtin_commit_graph_usage,
builtin_commit_graph_options);
git_config(git_default_config, NULL);
argc = parse_options(argc, argv, prefix,
builtin_commit_graph_options,
builtin_commit_graph_usage,
PARSE_OPT_STOP_AT_NON_OPTION);
save_commit_buffer = 0;
if (argc > 0) {
if (!strcmp(argv[0], "verify"))
return graph_verify(argc, argv);
if (!strcmp(argv[0], "write"))
return graph_write(argc, argv);
}
usage_with_options(builtin_commit_graph_usage,
builtin_commit_graph_options);
}
