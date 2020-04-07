



#include "cache.h"
#include "exec-cmd.h"
#include "gettext.h"
#include "strbuf.h"
#include "utf8.h"
#include "config.h"

#if !defined(NO_GETTEXT)
#include <locale.h>
#include <libintl.h>
#if defined(GIT_WINDOWS_NATIVE)

static const char *locale_charset(void)
{
const char *env = getenv("LC_ALL"), *dot;

if (!env || !*env)
env = getenv("LC_CTYPE");
if (!env || !*env)
env = getenv("LANG");

if (!env)
return "UTF-8";

dot = strchr(env, '.');
return !dot ? env : dot + 1;
}

#elif defined HAVE_LIBCHARSET_H
#include <libcharset.h>
#else
#include <langinfo.h>
#define locale_charset() nl_langinfo(CODESET)
#endif
#endif

static const char *charset;







const char *get_preferred_languages(void)
{
const char *retval;

retval = getenv("LANGUAGE");
if (retval && *retval)
return retval;

#if !defined(NO_GETTEXT)
retval = setlocale(LC_MESSAGES, NULL);
if (retval && *retval &&
strcmp(retval, "C") &&
strcmp(retval, "POSIX"))
return retval;
#endif

return NULL;
}

int use_gettext_poison(void)
{
static int poison_requested = -1;
if (poison_requested == -1)
poison_requested = git_env_bool("GIT_TEST_GETTEXT_POISON", 0);
return poison_requested;
}

#if !defined(NO_GETTEXT)
static int test_vsnprintf(const char *fmt, ...)
{
char buf[26];
int ret;
va_list ap;
va_start(ap, fmt);
ret = vsnprintf(buf, sizeof(buf), fmt, ap);
va_end(ap);
return ret;
}

static void init_gettext_charset(const char *domain)
{














































































setlocale(LC_CTYPE, "");
charset = locale_charset();
bind_textdomain_codeset(domain, charset);

if (test_vsnprintf("%.*s", 13, "David_K\345gedal") < 0)
setlocale(LC_CTYPE, "C");
}

void git_setup_gettext(void)
{
const char *podir = getenv(GIT_TEXT_DOMAIN_DIR_ENVIRONMENT);
char *p = NULL;

if (!podir)
podir = p = system_path(GIT_LOCALE_PATH);

use_gettext_poison(); 

if (!is_directory(podir)) {
free(p);
return;
}

bindtextdomain("git", podir);
setlocale(LC_MESSAGES, "");
setlocale(LC_TIME, "");
init_gettext_charset("git");
textdomain("git");

free(p);
}


int gettext_width(const char *s)
{
static int is_utf8 = -1;
if (is_utf8 == -1)
is_utf8 = is_utf8_locale();

return is_utf8 ? utf8_strwidth(s) : strlen(s);
}
#endif

int is_utf8_locale(void)
{
#if defined(NO_GETTEXT)
if (!charset) {
const char *env = getenv("LC_ALL");
if (!env || !*env)
env = getenv("LC_CTYPE");
if (!env || !*env)
env = getenv("LANG");
if (!env)
env = "";
if (strchr(env, '.'))
env = strchr(env, '.') + 1;
charset = xstrdup(env);
}
#endif
return is_encoding_utf8(charset);
}
