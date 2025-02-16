#include "cache.h"
#include "quote.h"
#include "exec-cmd.h"
#include "strbuf.h"
#include "run-command.h"
#include "alias.h"
#define COMMAND_DIR "git-shell-commands"
#define HELP_COMMAND COMMAND_DIR "/help"
#define NOLOGIN_COMMAND COMMAND_DIR "/no-interactive-login"
static int do_generic_cmd(const char *me, char *arg)
{
const char *my_argv[4];
setup_path();
if (!arg || !(arg = sq_dequote(arg)) || *arg == '-')
die("bad argument");
if (!skip_prefix(me, "git-", &me))
die("bad command");
my_argv[0] = me;
my_argv[1] = arg;
my_argv[2] = NULL;
return execv_git_cmd(my_argv);
}
static int is_valid_cmd_name(const char *cmd)
{
return cmd[strcspn(cmd, "./")] == '\0';
}
static char *make_cmd(const char *prog)
{
return xstrfmt("%s/%s", COMMAND_DIR, prog);
}
static void cd_to_homedir(void)
{
const char *home = getenv("HOME");
if (!home)
die("could not determine user's home directory; HOME is unset");
if (chdir(home) == -1)
die("could not chdir to user's home directory");
}
static void run_shell(void)
{
int done = 0;
static const char *help_argv[] = { HELP_COMMAND, NULL };
if (!access(NOLOGIN_COMMAND, F_OK)) {
const char *argv[] = { NOLOGIN_COMMAND, NULL };
int status;
status = run_command_v_opt(argv, 0);
if (status < 0)
exit(127);
exit(status);
}
run_command_v_opt(help_argv, RUN_SILENT_EXEC_FAILURE);
do {
struct strbuf line = STRBUF_INIT;
const char *prog;
char *full_cmd;
char *rawargs;
char *split_args;
const char **argv;
int code;
int count;
fprintf(stderr, "git> ");
if (strbuf_getline_lf(&line, stdin) == EOF) {
fprintf(stderr, "\n");
strbuf_release(&line);
break;
}
strbuf_trim(&line);
rawargs = strbuf_detach(&line, NULL);
split_args = xstrdup(rawargs);
count = split_cmdline(split_args, &argv);
if (count < 0) {
fprintf(stderr, "invalid command format '%s': %s\n", rawargs,
split_cmdline_strerror(count));
free(split_args);
free(rawargs);
continue;
}
prog = argv[0];
if (!strcmp(prog, "")) {
} else if (!strcmp(prog, "quit") || !strcmp(prog, "logout") ||
!strcmp(prog, "exit") || !strcmp(prog, "bye")) {
done = 1;
} else if (is_valid_cmd_name(prog)) {
full_cmd = make_cmd(prog);
argv[0] = full_cmd;
code = run_command_v_opt(argv, RUN_SILENT_EXEC_FAILURE);
if (code == -1 && errno == ENOENT) {
fprintf(stderr, "unrecognized command '%s'\n", prog);
}
free(full_cmd);
} else {
fprintf(stderr, "invalid command format '%s'\n", prog);
}
free(argv);
free(rawargs);
} while (!done);
}
static struct commands {
const char *name;
int (*exec)(const char *me, char *arg);
} cmd_list[] = {
{ "git-receive-pack", do_generic_cmd },
{ "git-upload-pack", do_generic_cmd },
{ "git-upload-archive", do_generic_cmd },
{ NULL },
};
int cmd_main(int argc, const char **argv)
{
char *prog;
const char **user_argv;
struct commands *cmd;
int count;
if (argc == 2 && !strcmp(argv[1], "cvs server")) {
argv--;
} else if (argc == 1) {
cd_to_homedir();
if (access(COMMAND_DIR, R_OK | X_OK) == -1) {
die("Interactive git shell is not enabled.\n"
"hint: ~/" COMMAND_DIR " should exist "
"and have read and execute access.");
}
run_shell();
exit(0);
} else if (argc != 3 || strcmp(argv[1], "-c")) {
die("Run with no arguments or with -c cmd");
}
prog = xstrdup(argv[2]);
if (!strncmp(prog, "git", 3) && isspace(prog[3]))
prog[3] = '-';
for (cmd = cmd_list ; cmd->name ; cmd++) {
int len = strlen(cmd->name);
char *arg;
if (strncmp(cmd->name, prog, len))
continue;
arg = NULL;
switch (prog[len]) {
case '\0':
arg = NULL;
break;
case ' ':
arg = prog + len + 1;
break;
default:
continue;
}
exit(cmd->exec(cmd->name, arg));
}
cd_to_homedir();
count = split_cmdline(prog, &user_argv);
if (count >= 0) {
if (is_valid_cmd_name(user_argv[0])) {
prog = make_cmd(user_argv[0]);
user_argv[0] = prog;
execv(user_argv[0], (char *const *) user_argv);
}
free(prog);
free(user_argv);
die("unrecognized command '%s'", argv[2]);
} else {
free(prog);
die("invalid command format '%s': %s", argv[2],
split_cmdline_strerror(count));
}
}
