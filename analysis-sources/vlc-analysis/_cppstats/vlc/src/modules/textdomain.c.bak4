



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include "modules/modules.h"

#if defined(ENABLE_NLS)
#include <libintl.h>
#include <vlc_charset.h>
#endif

int vlc_bindtextdomain (const char *domain)
{
#if defined (ENABLE_NLS)

char *upath = config_GetSysPath(VLC_LOCALE_DIR, NULL);
if (unlikely(upath == NULL))
return -1;

char *lpath = ToLocale(upath);
if (lpath == NULL || bindtextdomain (domain, lpath) == NULL)
{
LocaleFree(lpath);
fprintf (stderr, "%s: text domain not found in %s\n", domain, upath);
free (upath);
return -1;
}
LocaleFree(lpath);
free (upath);





if (bind_textdomain_codeset (PACKAGE_NAME, "UTF-8") == NULL)
{
fprintf (stderr, "%s: UTF-8 encoding not available\n", domain);

bindtextdomain (PACKAGE_NAME, "/DOES_NOT_EXIST");
return -1;
}






#else 
(void)domain;
#endif

return 0;
}




const char *vlc_gettext(const char *msgid)
{
#if defined(ENABLE_NLS)
if (likely(*msgid))
return dgettext (PACKAGE_NAME, msgid);
#endif
return msgid;
}

const char *vlc_ngettext(const char *msgid, const char *plural,
unsigned long n)
{
#if defined(ENABLE_NLS)
if (likely(*msgid))
return dngettext (PACKAGE_NAME, msgid, plural, n);
#endif
return ((n == 1) ? msgid : plural);
}
