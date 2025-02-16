#include "test-tool.h"
#include "cache.h"
#include "commit.h"
#include "diff.h"
#include "revision.h"
static void print_commit(struct commit *commit)
{
struct strbuf sb = STRBUF_INIT;
struct pretty_print_context ctx = {0};
ctx.date_mode.type = DATE_NORMAL;
format_commit_message(commit, " %m %s", &sb, &ctx);
printf("%s\n", sb.buf);
strbuf_release(&sb);
}
static int run_revision_walk(void)
{
struct rev_info rev;
struct commit *commit;
const char *argv[] = {NULL, "--all", NULL};
int argc = ARRAY_SIZE(argv) - 1;
int got_revision = 0;
repo_init_revisions(the_repository, &rev, NULL);
setup_revisions(argc, argv, &rev, NULL);
if (prepare_revision_walk(&rev))
die("revision walk setup failed");
while ((commit = get_revision(&rev)) != NULL) {
print_commit(commit);
got_revision = 1;
}
reset_revision_walk();
return got_revision;
}
int cmd__revision_walking(int argc, const char **argv)
{
if (argc < 2)
return 1;
setup_git_directory();
if (!strcmp(argv[1], "run-twice")) {
printf("1st\n");
if (!run_revision_walk())
return 1;
printf("2nd\n");
if (!run_revision_walk())
return 1;
return 0;
}
fprintf(stderr, "check usage\n");
return 1;
}
