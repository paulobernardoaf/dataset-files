









#include "test-tool.h"
#include "git-compat-util.h"
#include "cache.h"
#include "run-command.h"
#include "argv-array.h"
#include "strbuf.h"
#include "parse-options.h"
#include "string-list.h"
#include "thread-utils.h"
#include "wildmatch.h"
#include "gettext.h"
#include "parse-options.h"

static int number_callbacks;
static int parallel_next(struct child_process *cp,
struct strbuf *err,
void *cb,
void **task_cb)
{
struct child_process *d = cb;
if (number_callbacks >= 4)
return 0;

argv_array_pushv(&cp->args, d->argv);
strbuf_addstr(err, "preloaded output of a child\n");
number_callbacks++;
return 1;
}

static int no_job(struct child_process *cp,
struct strbuf *err,
void *cb,
void **task_cb)
{
strbuf_addstr(err, "no further jobs available\n");
return 0;
}

static int task_finished(int result,
struct strbuf *err,
void *pp_cb,
void *pp_task_cb)
{
strbuf_addstr(err, "asking for a quick stop\n");
return 1;
}

struct testsuite {
struct string_list tests, failed;
int next;
int quiet, immediate, verbose, verbose_log, trace, write_junit_xml;
};
#define TESTSUITE_INIT { STRING_LIST_INIT_DUP, STRING_LIST_INIT_DUP, -1, 0, 0, 0, 0, 0, 0 }


static int next_test(struct child_process *cp, struct strbuf *err, void *cb,
void **task_cb)
{
struct testsuite *suite = cb;
const char *test;
if (suite->next >= suite->tests.nr)
return 0;

test = suite->tests.items[suite->next++].string;
argv_array_pushl(&cp->args, "sh", test, NULL);
if (suite->quiet)
argv_array_push(&cp->args, "--quiet");
if (suite->immediate)
argv_array_push(&cp->args, "-i");
if (suite->verbose)
argv_array_push(&cp->args, "-v");
if (suite->verbose_log)
argv_array_push(&cp->args, "-V");
if (suite->trace)
argv_array_push(&cp->args, "-x");
if (suite->write_junit_xml)
argv_array_push(&cp->args, "--write-junit-xml");

strbuf_addf(err, "Output of '%s':\n", test);
*task_cb = (void *)test;

return 1;
}

static int test_finished(int result, struct strbuf *err, void *cb,
void *task_cb)
{
struct testsuite *suite = cb;
const char *name = (const char *)task_cb;

if (result)
string_list_append(&suite->failed, name);

strbuf_addf(err, "%s: '%s'\n", result ? "FAIL" : "SUCCESS", name);

return 0;
}

static int test_failed(struct strbuf *out, void *cb, void *task_cb)
{
struct testsuite *suite = cb;
const char *name = (const char *)task_cb;

string_list_append(&suite->failed, name);
strbuf_addf(out, "FAILED TO START: '%s'\n", name);

return 0;
}

static const char * const testsuite_usage[] = {
"test-run-command testsuite [<options>] [<pattern>...]",
NULL
};

static int testsuite(int argc, const char **argv)
{
struct testsuite suite = TESTSUITE_INIT;
int max_jobs = 1, i, ret;
DIR *dir;
struct dirent *d;
struct option options[] = {
OPT_BOOL('i', "immediate", &suite.immediate,
"stop at first failed test case(s)"),
OPT_INTEGER('j', "jobs", &max_jobs, "run <N> jobs in parallel"),
OPT_BOOL('q', "quiet", &suite.quiet, "be terse"),
OPT_BOOL('v', "verbose", &suite.verbose, "be verbose"),
OPT_BOOL('V', "verbose-log", &suite.verbose_log,
"be verbose, redirected to a file"),
OPT_BOOL('x', "trace", &suite.trace, "trace shell commands"),
OPT_BOOL(0, "write-junit-xml", &suite.write_junit_xml,
"write JUnit-style XML files"),
OPT_END()
};

memset(&suite, 0, sizeof(suite));
suite.tests.strdup_strings = suite.failed.strdup_strings = 1;

argc = parse_options(argc, argv, NULL, options,
testsuite_usage, PARSE_OPT_STOP_AT_NON_OPTION);

if (max_jobs <= 0)
max_jobs = online_cpus();

dir = opendir(".");
if (!dir)
die("Could not open the current directory");
while ((d = readdir(dir))) {
const char *p = d->d_name;

if (*p != 't' || !isdigit(p[1]) || !isdigit(p[2]) ||
!isdigit(p[3]) || !isdigit(p[4]) || p[5] != '-' ||
!ends_with(p, ".sh"))
continue;


if (!argc) {
string_list_append(&suite.tests, p);
continue;
}

for (i = 0; i < argc; i++)
if (!wildmatch(argv[i], p, 0)) {
string_list_append(&suite.tests, p);
break;
}
}
closedir(dir);

if (!suite.tests.nr)
die("No tests match!");
if (max_jobs > suite.tests.nr)
max_jobs = suite.tests.nr;

fprintf(stderr, "Running %d tests (%d at a time)\n",
suite.tests.nr, max_jobs);

ret = run_processes_parallel(max_jobs, next_test, test_failed,
test_finished, &suite);

if (suite.failed.nr > 0) {
ret = 1;
fprintf(stderr, "%d tests failed:\n\n", suite.failed.nr);
for (i = 0; i < suite.failed.nr; i++)
fprintf(stderr, "\t%s\n", suite.failed.items[i].string);
}

string_list_clear(&suite.tests, 0);
string_list_clear(&suite.failed, 0);

return !!ret;
}

static uint64_t my_random_next = 1234;

static uint64_t my_random(void)
{
uint64_t res = my_random_next;
my_random_next = my_random_next * 1103515245 + 12345;
return res;
}

static int quote_stress_test(int argc, const char **argv)
{






char special[] = ".?*\\^_\"'`{}()[]<>@~&+:;$%"; 
int i, j, k, trials = 100, skip = 0, msys2 = 0;
struct strbuf out = STRBUF_INIT;
struct argv_array args = ARGV_ARRAY_INIT;
struct option options[] = {
OPT_INTEGER('n', "trials", &trials, "Number of trials"),
OPT_INTEGER('s', "skip", &skip, "Skip <n> trials"),
OPT_BOOL('m', "msys2", &msys2, "Test quoting for MSYS2's sh"),
OPT_END()
};
const char * const usage[] = {
"test-tool run-command quote-stress-test <options>",
NULL
};

argc = parse_options(argc, argv, NULL, options, usage, 0);

setenv("MSYS_NO_PATHCONV", "1", 0);

for (i = 0; i < trials; i++) {
struct child_process cp = CHILD_PROCESS_INIT;
size_t arg_count, arg_offset;
int ret = 0;

argv_array_clear(&args);
if (msys2)
argv_array_pushl(&args, "sh", "-c",
"printf %s\\\\0 \"$@\"", "skip", NULL);
else
argv_array_pushl(&args, "test-tool", "run-command",
"quote-echo", NULL);
arg_offset = args.argc;

if (argc > 0) {
trials = 1;
arg_count = argc;
for (j = 0; j < arg_count; j++)
argv_array_push(&args, argv[j]);
} else {
arg_count = 1 + (my_random() % 5);
for (j = 0; j < arg_count; j++) {
char buf[20];
size_t min_len = 1;
size_t arg_len = min_len +
(my_random() % (ARRAY_SIZE(buf) - min_len));

for (k = 0; k < arg_len; k++)
buf[k] = special[my_random() %
ARRAY_SIZE(special)];
buf[arg_len] = '\0';

argv_array_push(&args, buf);
}
}

if (i < skip)
continue;

cp.argv = args.argv;
strbuf_reset(&out);
if (pipe_command(&cp, NULL, 0, &out, 0, NULL, 0) < 0)
return error("Failed to spawn child process");

for (j = 0, k = 0; j < arg_count; j++) {
const char *arg = args.argv[j + arg_offset];

if (strcmp(arg, out.buf + k))
ret = error("incorrectly quoted arg: '%s', "
"echoed back as '%s'",
arg, out.buf + k);
k += strlen(out.buf + k) + 1;
}

if (k != out.len)
ret = error("got %d bytes, but consumed only %d",
(int)out.len, (int)k);

if (ret) {
fprintf(stderr, "Trial #%d failed. Arguments:\n", i);
for (j = 0; j < arg_count; j++)
fprintf(stderr, "arg #%d: '%s'\n",
(int)j, args.argv[j + arg_offset]);

strbuf_release(&out);
argv_array_clear(&args);

return ret;
}

if (i && (i % 100) == 0)
fprintf(stderr, "Trials completed: %d\n", (int)i);
}

strbuf_release(&out);
argv_array_clear(&args);

return 0;
}

static int quote_echo(int argc, const char **argv)
{
while (argc > 1) {
fwrite(argv[1], strlen(argv[1]), 1, stdout);
fputc('\0', stdout);
argv++;
argc--;
}

return 0;
}

static int inherit_handle(const char *argv0)
{
struct child_process cp = CHILD_PROCESS_INIT;
char path[PATH_MAX];
int tmp;


xsnprintf(path, sizeof(path), "out-XXXXXX");
tmp = xmkstemp(path);

argv_array_pushl(&cp.args,
"test-tool", argv0, "inherited-handle-child", NULL);
cp.in = -1;
cp.no_stdout = cp.no_stderr = 1;
if (start_command(&cp) < 0)
die("Could not start child process");


close(tmp);
if (unlink(path))
die("Could not delete '%s'", path);

if (close(cp.in) < 0 || finish_command(&cp) < 0)
die("Child did not finish");

return 0;
}

static int inherit_handle_child(void)
{
struct strbuf buf = STRBUF_INIT;

if (strbuf_read(&buf, 0, 0) < 0)
die("Could not read stdin");
printf("Received %s\n", buf.buf);
strbuf_release(&buf);

return 0;
}

int cmd__run_command(int argc, const char **argv)
{
struct child_process proc = CHILD_PROCESS_INIT;
int jobs;

if (argc > 1 && !strcmp(argv[1], "testsuite"))
exit(testsuite(argc - 1, argv + 1));
if (!strcmp(argv[1], "inherited-handle"))
exit(inherit_handle(argv[0]));
if (!strcmp(argv[1], "inherited-handle-child"))
exit(inherit_handle_child());

if (argc >= 2 && !strcmp(argv[1], "quote-stress-test"))
return !!quote_stress_test(argc - 1, argv + 1);

if (argc >= 2 && !strcmp(argv[1], "quote-echo"))
return !!quote_echo(argc - 1, argv + 1);

if (argc < 3)
return 1;
while (!strcmp(argv[1], "env")) {
if (!argv[2])
die("env specifier without a value");
argv_array_push(&proc.env_array, argv[2]);
argv += 2;
argc -= 2;
}
if (argc < 3)
return 1;
proc.argv = (const char **)argv + 2;

if (!strcmp(argv[1], "start-command-ENOENT")) {
if (start_command(&proc) < 0 && errno == ENOENT)
return 0;
fprintf(stderr, "FAIL %s\n", argv[1]);
return 1;
}
if (!strcmp(argv[1], "run-command"))
exit(run_command(&proc));

jobs = atoi(argv[2]);
proc.argv = (const char **)argv + 3;

if (!strcmp(argv[1], "run-command-parallel"))
exit(run_processes_parallel(jobs, parallel_next,
NULL, NULL, &proc));

if (!strcmp(argv[1], "run-command-abort"))
exit(run_processes_parallel(jobs, parallel_next,
NULL, task_finished, &proc));

if (!strcmp(argv[1], "run-command-no-jobs"))
exit(run_processes_parallel(jobs, no_job,
NULL, task_finished, &proc));

fprintf(stderr, "check usage\n");
return 1;
}
