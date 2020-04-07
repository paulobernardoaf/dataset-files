#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <string.h>
char *strsep( char **ppsz_string, const char *psz_delimiters )
{
char *psz_string = *ppsz_string;
if( !psz_string )
return NULL;
char *p = strpbrk( psz_string, psz_delimiters );
if( !p )
{
*ppsz_string = NULL;
return psz_string;
}
*p++ = '\0';
*ppsz_string = p;
return psz_string;
}
