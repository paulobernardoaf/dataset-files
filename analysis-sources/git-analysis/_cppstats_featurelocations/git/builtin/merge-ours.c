








#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "git-compat-util.h"
#include "builtin.h"
#include "diff.h"

static const char builtin_merge_ours_usage[] =
"git merge-ours <base>... -- HEAD <remote>...";

int cmd_merge_ours(int argc, const char **argv, const char *prefix)
{
if (argc == 2 && !strcmp(argv[1], "-h"))
usage(builtin_merge_ours_usage);






if (read_cache() < 0)
die_errno("read_cache failed");
if (index_differs_from(the_repository, "HEAD", NULL, 0))
exit(2);
exit(0);
}
