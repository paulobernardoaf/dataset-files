




#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "cache.h"
#include "config.h"
#include "diff.h"
#include "commit.h"
#include "revision.h"
#include "builtin.h"
#include "submodule.h"

static const char diff_files_usage[] =
"git diff-files [-q] [-0 | -1 | -2 | -3 | -c | --cc] [<common-diff-options>] [<path>...]"
COMMON_DIFF_OPTIONS_HELP;

int cmd_diff_files(int argc, const char **argv, const char *prefix)
{
struct rev_info rev;
int result;
unsigned options = 0;

if (argc == 2 && !strcmp(argv[1], "-h"))
usage(diff_files_usage);

git_config(git_diff_basic_config, NULL); 
repo_init_revisions(the_repository, &rev, prefix);
rev.abbrev = 0;
precompose_argv(argc, argv);

argc = setup_revisions(argc, argv, &rev, NULL);
while (1 < argc && argv[1][0] == '-') {
if (!strcmp(argv[1], "--base"))
rev.max_count = 1;
else if (!strcmp(argv[1], "--ours"))
rev.max_count = 2;
else if (!strcmp(argv[1], "--theirs"))
rev.max_count = 3;
else if (!strcmp(argv[1], "-q"))
options |= DIFF_SILENT_ON_REMOVED;
else
usage(diff_files_usage);
argv++; argc--;
}
if (!rev.diffopt.output_format)
rev.diffopt.output_format = DIFF_FORMAT_RAW;






if (rev.pending.nr ||
rev.min_age != -1 || rev.max_age != -1 ||
3 < rev.max_count)
usage(diff_files_usage);






if (rev.max_count == -1 && !rev.combine_merges &&
(rev.diffopt.output_format & DIFF_FORMAT_PATCH))
rev.combine_merges = rev.dense_combined_merges = 1;

if (read_cache_preload(&rev.diffopt.pathspec) < 0) {
perror("read_cache_preload");
return -1;
}
result = run_diff_files(&rev, options);
return diff_result_code(&rev.diffopt, result);
}
