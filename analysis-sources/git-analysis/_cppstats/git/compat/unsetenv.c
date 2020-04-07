#include "../git-compat-util.h"
void gitunsetenv (const char *name)
{
#if !defined(__MINGW32__)
extern char **environ;
#endif
int src, dst;
size_t nmln;
nmln = strlen(name);
for (src = dst = 0; environ[src]; ++src) {
size_t enln;
enln = strlen(environ[src]);
if (enln > nmln) {
if (0 == strncmp (environ[src], name, nmln)
&& '=' == environ[src][nmln])
continue;
}
environ[dst] = environ[src];
++dst;
}
environ[dst] = NULL;
}
