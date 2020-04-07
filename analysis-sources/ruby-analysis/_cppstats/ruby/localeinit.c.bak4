










#include "ruby/encoding.h"
#include "internal.h"
#include "encindex.h"
#if defined(__CYGWIN__)
#include <windows.h>
#endif
#if defined(HAVE_LANGINFO_H)
#include <langinfo.h>
#endif

#if defined _WIN32 || defined __CYGWIN__
#define SIZEOF_CP_NAME ((sizeof(UINT) * 8 / 3) + 4)
#define CP_FORMAT(buf, codepage) snprintf(buf, sizeof(buf), "CP%u", (codepage))

extern UINT ruby_w32_codepage[2];
#endif

#if !defined(NO_LOCALE_CHARMAP)
#if defined _WIN32 || defined __CYGWIN__ || defined HAVE_LANGINFO_H
#define NO_LOCALE_CHARMAP 0
#else
#define NO_LOCALE_CHARMAP 1
#endif
#endif

#if !NO_LOCALE_CHARMAP
static VALUE
locale_charmap(VALUE (*conv)(const char *))
{
const char *codeset = 0;
#if defined _WIN32 || defined __CYGWIN__
char cp[SIZEOF_CP_NAME];
#if defined(__CYGWIN__)
const char *nl_langinfo_codeset(void);
codeset = nl_langinfo_codeset();
#endif
if (!codeset) {
UINT codepage = ruby_w32_codepage[0];
if (!codepage) codepage = GetConsoleCP();
if (!codepage) codepage = GetACP();
CP_FORMAT(cp, codepage);
codeset = cp;
}
#elif defined HAVE_LANGINFO_H
codeset = nl_langinfo(CODESET);
ASSUME(codeset);
#else
#error locale_charmap() is not implemented
#endif
return (*conv)(codeset);
}
#endif


























VALUE
rb_locale_charmap(VALUE klass)
{
#if NO_LOCALE_CHARMAP
return rb_usascii_str_new_cstr("US-ASCII");
#else
return locale_charmap(rb_usascii_str_new_cstr);
#endif
}

#if !NO_LOCALE_CHARMAP
static VALUE
enc_find_index(const char *name)
{
return (VALUE)rb_enc_find_index(name);
}
#endif

int
rb_locale_charmap_index(void)
{
#if NO_LOCALE_CHARMAP
return ENCINDEX_US_ASCII;
#else
return (int)locale_charmap(enc_find_index);
#endif
}

int
Init_enc_set_filesystem_encoding(void)
{
int idx;
#if NO_LOCALE_CHARMAP
idx = ENCINDEX_US_ASCII;
#elif defined _WIN32
char cp[SIZEOF_CP_NAME];
const UINT codepage = ruby_w32_codepage[1] ? ruby_w32_codepage[1] :
AreFileApisANSI() ? GetACP() : GetOEMCP();
CP_FORMAT(cp, codepage);
idx = rb_enc_find_index(cp);
if (idx < 0) idx = ENCINDEX_ASCII;
#elif defined __CYGWIN__
idx = ENCINDEX_UTF_8;
#else
idx = rb_enc_to_index(rb_default_external_encoding());
#endif
return idx;
}
