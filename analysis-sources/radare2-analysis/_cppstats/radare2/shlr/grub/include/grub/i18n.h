#include <grub/symbol.h>
extern const char *(*EXPORT_VAR(grub_gettext)) (const char *s);
#if (defined(ENABLE_NLS) && ENABLE_NLS)
#if defined(GRUB_UTIL)
#include <locale.h>
#include <libintl.h>
#endif 
#else 
#if defined(GRUB_UTIL)
#define gettext(Msgid) ((const char *) (Msgid))
#else
#define grub_gettext(str) ((const char *) (str))
#endif 
#endif 
#if defined(GRUB_UTIL)
#define _(str) gettext(str)
#else
#define _(str) grub_gettext(str)
#endif 
#define N_(str) str
