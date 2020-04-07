#include "test-tool.h"
#include "cache.h"
#include "argv-array.h"
#include "run-command.h"
#include "exec-cmd.h"
#include "config.h"

typedef int(fn_unit_test)(int argc, const char **argv);

struct unit_test {
fn_unit_test *ut_fn;
const char *ut_name;
const char *ut_usage;
};

#define MyOk 0
#define MyError 1

static int get_i(int *p_value, const char *data)
{
char *endptr;

if (!data || !*data)
return MyError;

*p_value = strtol(data, &endptr, 10);
if (*endptr || errno == ERANGE)
return MyError;

return MyOk;
}















static int ut_001return(int argc, const char **argv)
{
int rc;

if (get_i(&rc, argv[0]))
die("expect <exit_code>");

return rc;
}











static int ut_002exit(int argc, const char **argv)
{
int rc;

if (get_i(&rc, argv[0]))
die("expect <exit_code>");

exit(rc);
}











static int ut_003error(int argc, const char **argv)
{
int k;

if (!argv[0] || !*argv[0])
die("expect <error_message>");

for (k = 0; k < argc; k++)
error("%s", argv[k]);

return 0;
}


































static int ut_004child(int argc, const char **argv)
{
int result;





if (!argc)
return 0;

result = run_command_v_opt(argv, 0);
exit(result);
}






















static int ut_005exec(int argc, const char **argv)
{
int result;

if (!argc)
return 0;

result = execv_git_cmd(argv);
return result;
}

static int ut_006data(int argc, const char **argv)
{
const char *usage_error =
"expect <cat0> <k0> <v0> [<cat1> <k1> <v1> [...]]";

if (argc % 3 != 0)
die("%s", usage_error);

while (argc) {
if (!argv[0] || !*argv[0] || !argv[1] || !*argv[1] ||
!argv[2] || !*argv[2])
die("%s", usage_error);

trace2_data_string(argv[0], the_repository, argv[1], argv[2]);
argv += 3;
argc -= 3;
}

return 0;
}







#define USAGE_PREFIX "test-tool trace2"


static struct unit_test ut_table[] = {
{ ut_001return, "001return", "<exit_code>" },
{ ut_002exit, "002exit", "<exit_code>" },
{ ut_003error, "003error", "<error_message>+" },
{ ut_004child, "004child", "[<child_command_line>]" },
{ ut_005exec, "005exec", "<git_command_args>" },
{ ut_006data, "006data", "[<category> <key> <value>]+" },
};



#define for_each_ut(k, ut_k) for (k = 0, ut_k = &ut_table[k]; k < ARRAY_SIZE(ut_table); k++, ut_k = &ut_table[k])





static int print_usage(void)
{
int k;
struct unit_test *ut_k;

fprintf(stderr, "usage:\n");
for_each_ut (k, ut_k)
fprintf(stderr, "\t%s %s %s\n", USAGE_PREFIX, ut_k->ut_name,
ut_k->ut_usage);

return 129;
}




















int cmd__trace2(int argc, const char **argv)
{
int k;
struct unit_test *ut_k;

argc--; 
argv++;

if (argc)
for_each_ut (k, ut_k)
if (!strcmp(argv[0], ut_k->ut_name))
return ut_k->ut_fn(argc - 1, argv + 1);

return print_usage();
}
