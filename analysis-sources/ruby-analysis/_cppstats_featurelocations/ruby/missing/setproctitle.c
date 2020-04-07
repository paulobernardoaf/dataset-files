

































#include "ruby.h"
#include "ruby/util.h"
#define compat_init_setproctitle ruby_init_setproctitle
RUBY_FUNC_EXPORTED void ruby_init_setproctitle(int argc, char *argv[]);

#if !defined(HAVE_SETPROCTITLE)

#include <stdarg.h>
#include <stdlib.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_SYS_PSTAT_H)
#include <sys/pstat.h>
#endif
#include <string.h>

#if defined(__APPLE__)
#if defined(HAVE_CRT_EXTERNS_H)
#include <crt_externs.h>
#undef environ
#define environ (*_NSGetEnviron())
#else
#include "crt_externs.h"
#endif
#endif

#define SPT_NONE 0 
#define SPT_PSTAT 1 
#define SPT_REUSEARGV 2 

#if !defined(SPT_TYPE)
#define SPT_TYPE SPT_NONE
#endif

#if !defined(SPT_PADCHAR)
#define SPT_PADCHAR '\0'
#endif

#if SPT_TYPE == SPT_REUSEARGV
static char *argv_start = NULL;
static size_t argv_env_len = 0;
static size_t argv_len = 0;
static char **argv1_addr = NULL;
#endif

#endif 

void
compat_init_setproctitle(int argc, char *argv[])
{
#if defined(SPT_TYPE) && SPT_TYPE == SPT_REUSEARGV
extern char **environ;
char *lastargv = NULL;
char *lastenvp = NULL;
char **envp = environ;
int i;







if (argc == 0 || argv[0] == NULL)
return;


for (i = 0; envp[i] != NULL; i++)
;
if ((environ = calloc(i + 1, sizeof(*environ))) == NULL) {
environ = envp; 
return;
}





for (i = 0; i < argc; i++) {
if (lastargv == NULL || lastargv + 1 == argv[i])
lastargv = argv[i] + strlen(argv[i]);
}
lastenvp = lastargv;
for (i = 0; envp[i] != NULL; i++) {
if (lastenvp + 1 == envp[i])
lastenvp = envp[i] + strlen(envp[i]);
}



argv1_addr = &argv[1];
argv_start = argv[0];
argv_len = lastargv - argv[0];
argv_env_len = lastenvp - argv[0];

for (i = 0; envp[i] != NULL; i++)
environ[i] = ruby_strdup(envp[i]);
environ[i] = NULL;
#endif 
}

#if !defined(HAVE_SETPROCTITLE)
void
setproctitle(const char *fmt, ...)
{
#if SPT_TYPE != SPT_NONE
va_list ap;
char ptitle[1024];
size_t len;
size_t argvlen;
#if SPT_TYPE == SPT_PSTAT
union pstun pst;
#endif

#if SPT_TYPE == SPT_REUSEARGV
if (argv_env_len <= 0)
return;
#endif


va_start(ap, fmt);
vsnprintf(ptitle, sizeof(ptitle), fmt, ap);
va_end(ap);

#if SPT_TYPE == SPT_PSTAT
pst.pst_command = ptitle;
pstat(PSTAT_SETCMD, pst, strlen(ptitle), 0, 0);
#elif SPT_TYPE == SPT_REUSEARGV
len = strlcpy(argv_start, ptitle, argv_env_len);
argvlen = len > argv_len ? argv_env_len : argv_len;
for(; len < argvlen; len++)
argv_start[len] = SPT_PADCHAR;

*argv1_addr = NULL;
#endif

#endif 
}

#endif 
