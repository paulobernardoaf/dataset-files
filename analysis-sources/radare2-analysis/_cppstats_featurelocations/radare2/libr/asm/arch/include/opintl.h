





















#if defined(ENABLE_NLS)
#include <libintl.h>













#define _(String) dgettext (PACKAGE, String)
#if defined(gettext_noop)
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#else
#define gettext(Msgid) (Msgid)
#define dgettext(Domainname, Msgid) (Msgid)
#define dcgettext(Domainname, Msgid, Category) (Msgid)
#define textdomain(Domainname) while (0) 
#define bindtextdomain(Domainname, Dirname) while (0) 
#define _(String) (String)
#define N_(String) (String)
#endif
