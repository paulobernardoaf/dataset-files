#include "internal.h"
#if defined HAVE_DLADDR
#include <dlfcn.h>
#endif
#if defined HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
static void* stub_options(int argc, char **argv);
#define ruby_options stub_options
#include <main.c>
#undef ruby_options
void *
stub_options(int argc, char **argv)
{
char xflag[] = "-x";
char *xargv[4] = {NULL, xflag};
char *cmd = argv[0];
void *ret;
#if defined __CYGWIN__ || defined _WIN32
#elif defined __linux__
{
char selfexe[MAXPATHLEN];
ssize_t len = readlink("/proc/self/exe", selfexe, sizeof(selfexe));
if (len < 0) {
perror("readlink(\"/proc/self/exe\")");
return NULL;
}
selfexe[len] = '\0';
cmd = selfexe;
}
#elif defined HAVE_DLADDR
{
Dl_info dli;
if (!dladdr(stub_options, &dli)) {
perror("dladdr");
return NULL;
}
cmd = (char *)dli.dli_fname;
}
#endif
#if !defined(HAVE_SETPROCTITLE)
ruby_init_setproctitle(argc, argv);
#endif
xargv[2] = cmd;
ret = ruby_options(3, xargv);
ruby_set_argv(argc - 1, argv + 1);
return ret;
}
