











#include "tuklib_progname.h"
#include <string.h>


#if !HAVE_DECL_PROGRAM_INVOCATION_NAME
char *progname = NULL;
#endif


extern void
tuklib_progname_init(char **argv)
{
#if defined(TUKLIB_DOSLIKE)





char *p = argv[0] + strlen(argv[0]);
while (argv[0] < p && p[-1] != '/' && p[-1] != '\\')
--p;

argv[0] = p;


p = strrchr(p, '.');
if (p != NULL)
*p = '\0';


for (p = argv[0]; *p != '\0'; ++p)
if (*p >= 'A' && *p <= 'Z')
*p = *p - 'A' + 'a';
#endif

progname = argv[0];
return;
}
