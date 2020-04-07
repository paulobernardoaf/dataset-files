#include "cache.h"
#include "exec-cmd.h"
#include "attr.h"
static void restore_sigpipe_to_default(void)
{
sigset_t unblock;
sigemptyset(&unblock);
sigaddset(&unblock, SIGPIPE);
sigprocmask(SIG_UNBLOCK, &unblock, NULL);
signal(SIGPIPE, SIG_DFL);
}
int main(int argc, const char **argv)
{
int result;
trace2_initialize_clock();
sanitize_stdfds();
restore_sigpipe_to_default();
git_resolve_executable_dir(argv[0]);
git_setup_gettext();
initialize_the_repository();
attr_start();
trace2_initialize();
trace2_cmd_start(argv);
trace2_collect_process_info(TRACE2_PROCESS_INFO_STARTUP);
result = cmd_main(argc, argv);
trace2_cmd_exit(result);
return result;
}
