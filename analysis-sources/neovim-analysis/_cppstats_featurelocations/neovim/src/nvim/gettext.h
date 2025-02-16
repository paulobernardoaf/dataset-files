#if !defined(NVIM_GETTEXT_H)
#define NVIM_GETTEXT_H

#if defined(HAVE_WORKING_LIBINTL)
#include <libintl.h>
#define _(x) gettext((char *)(x))

#if defined(gettext_noop)
#define N_(x) gettext_noop(x)
#else
#define N_(x) x
#endif
#define NGETTEXT(x, xs, n) ngettext(x, xs, n)


#if defined(__APPLE__) && defined(setlocale)
#undef setlocale
#endif
#else
#define _(x) ((char *)(x))
#define N_(x) x
#define NGETTEXT(x, xs, n) ((n) == 1 ? (x) : (xs))
#define bindtextdomain(x, y) 
#define bind_textdomain_codeset(x, y) 
#define textdomain(x) 
#endif

#endif 
