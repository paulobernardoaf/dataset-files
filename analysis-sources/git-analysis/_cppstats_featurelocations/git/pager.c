#include "cache.h"
#include "config.h"
#include "run-command.h"
#include "sigchain.h"
#include "alias.h"

#if !defined(DEFAULT_PAGER)
#define DEFAULT_PAGER "less"
#endif

static struct child_process pager_process = CHILD_PROCESS_INIT;
static const char *pager_program;

static void wait_for_pager(int in_signal)
{
if (!in_signal) {
fflush(stdout);
fflush(stderr);
}

close(1);
close(2);
if (in_signal)
finish_command_in_signal(&pager_process);
else
finish_command(&pager_process);
}

static void wait_for_pager_atexit(void)
{
wait_for_pager(0);
}

static void wait_for_pager_signal(int signo)
{
wait_for_pager(1);
sigchain_pop(signo);
raise(signo);
}

static int core_pager_config(const char *var, const char *value, void *data)
{
if (!strcmp(var, "core.pager"))
return git_config_string(&pager_program, var, value);
return 0;
}

const char *git_pager(int stdout_is_tty)
{
const char *pager;

if (!stdout_is_tty)
return NULL;

pager = getenv("GIT_PAGER");
if (!pager) {
if (!pager_program)
read_early_config(core_pager_config, NULL);
pager = pager_program;
}
if (!pager)
pager = getenv("PAGER");
if (!pager)
pager = DEFAULT_PAGER;
if (!*pager || !strcmp(pager, "cat"))
pager = NULL;

return pager;
}

static void setup_pager_env(struct argv_array *env)
{
const char **argv;
int i;
char *pager_env = xstrdup(PAGER_ENV);
int n = split_cmdline(pager_env, &argv);

if (n < 0)
die("malformed build-time PAGER_ENV: %s",
split_cmdline_strerror(n));

for (i = 0; i < n; i++) {
char *cp = strchr(argv[i], '=');

if (!cp)
die("malformed build-time PAGER_ENV");

*cp = '\0';
if (!getenv(argv[i])) {
*cp = '=';
argv_array_push(env, argv[i]);
}
}
free(pager_env);
free(argv);
}

void prepare_pager_args(struct child_process *pager_process, const char *pager)
{
argv_array_push(&pager_process->args, pager);
pager_process->use_shell = 1;
setup_pager_env(&pager_process->env_array);
pager_process->trace2_child_class = "pager";
}

void setup_pager(void)
{
const char *pager = git_pager(isatty(1));

if (!pager)
return;






{
char buf[64];
xsnprintf(buf, sizeof(buf), "%d", term_columns());
setenv("COLUMNS", buf, 0);
}

setenv("GIT_PAGER_IN_USE", "true", 1);


prepare_pager_args(&pager_process, pager);
pager_process.in = -1;
argv_array_push(&pager_process.env_array, "GIT_PAGER_IN_USE");
if (start_command(&pager_process))
return;


dup2(pager_process.in, 1);
if (isatty(2))
dup2(pager_process.in, 2);
close(pager_process.in);


sigchain_push_common(wait_for_pager_signal);
atexit(wait_for_pager_atexit);
}

int pager_in_use(void)
{
return git_env_bool("GIT_PAGER_IN_USE", 0);
}






int term_columns(void)
{
static int term_columns_at_startup;

char *col_string;
int n_cols;

if (term_columns_at_startup)
return term_columns_at_startup;

term_columns_at_startup = 80;

col_string = getenv("COLUMNS");
if (col_string && (n_cols = atoi(col_string)) > 0)
term_columns_at_startup = n_cols;
#if defined(TIOCGWINSZ)
else {
struct winsize ws;
if (!ioctl(1, TIOCGWINSZ, &ws) && ws.ws_col)
term_columns_at_startup = ws.ws_col;
}
#endif

return term_columns_at_startup;
}




void term_clear_line(void)
{
if (is_terminal_dumb())





fprintf(stderr, "\r%*s\r", term_columns(), "");
else




fputs("\r\033[K", stderr);
}




int decimal_width(uintmax_t number)
{
int width;

for (width = 1; number >= 10; width++)
number /= 10;
return width;
}

struct pager_command_config_data {
const char *cmd;
int want;
char *value;
};

static int pager_command_config(const char *var, const char *value, void *vdata)
{
struct pager_command_config_data *data = vdata;
const char *cmd;

if (skip_prefix(var, "pager.", &cmd) && !strcmp(cmd, data->cmd)) {
int b = git_parse_maybe_bool(value);
if (b >= 0)
data->want = b;
else {
data->want = 1;
data->value = xstrdup(value);
}
}

return 0;
}


int check_pager_config(const char *cmd)
{
struct pager_command_config_data data;

data.cmd = cmd;
data.want = -1;
data.value = NULL;

read_early_config(pager_command_config, &data);

if (data.value)
pager_program = data.value;
return data.want;
}
