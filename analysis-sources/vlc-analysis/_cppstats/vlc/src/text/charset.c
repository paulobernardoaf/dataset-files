#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#if !defined _WIN32
#include <locale.h>
#else
#include <windows.h>
#endif
#if defined(__APPLE__)
#include <string.h>
#include <xlocale.h>
#endif
#include "libvlc.h"
#include <vlc_charset.h>
double us_strtod( const char *str, char **end )
{
locale_t loc = newlocale (LC_NUMERIC_MASK, "C", NULL);
locale_t oldloc = uselocale (loc);
double res = strtod (str, end);
if (loc != (locale_t)0)
{
uselocale (oldloc);
freelocale (loc);
}
return res;
}
float us_strtof( const char *str, char **end )
{
locale_t loc = newlocale (LC_NUMERIC_MASK, "C", NULL);
locale_t oldloc = uselocale (loc);
float res = strtof (str, end);
if (loc != (locale_t)0)
{
uselocale (oldloc);
freelocale (loc);
}
return res;
}
double us_atof( const char *str )
{
return us_strtod( str, NULL );
}
int us_vasprintf( char **ret, const char *format, va_list ap )
{
locale_t loc = newlocale( LC_NUMERIC_MASK, "C", NULL );
locale_t oldloc = uselocale( loc );
int i_rc = vasprintf( ret, format, ap );
if ( loc != (locale_t)0 )
{
uselocale( oldloc );
freelocale( loc );
}
return i_rc;
}
int us_asprintf( char **ret, const char *format, ... )
{
va_list ap;
int i_rc;
va_start( ap, format );
i_rc = us_vasprintf( ret, format, ap );
va_end( ap );
return i_rc;
}
