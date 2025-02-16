#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../libvlc.h"
#include <dirent.h> 
#include <CoreFoundation/CoreFoundation.h>
#include <locale.h>
void system_Init(void)
{
#if defined(ENABLE_NLS)
if( NULL == getenv("LANG") )
{
CFArrayRef all_locales, preferred_locales;
char psz_locale[50];
all_locales = CFLocaleCopyAvailableLocaleIdentifiers();
preferred_locales = CFBundleCopyLocalizationsForPreferences( all_locales, NULL );
if ( preferred_locales )
{
if ( CFArrayGetCount( preferred_locales ) )
{
CFStringRef user_language_string_ref = CFArrayGetValueAtIndex( preferred_locales, 0 );
CFStringGetCString( user_language_string_ref, psz_locale, sizeof(psz_locale), kCFStringEncodingUTF8 );
setenv( "LANG", psz_locale, 1 );
}
CFRelease( preferred_locales );
}
CFRelease( all_locales );
}
#endif
}
void system_Configure( libvlc_int_t *p_this,
int i_argc, const char *const ppsz_argv[] )
{
(void)p_this;
(void)i_argc;
(void)ppsz_argv;
}
