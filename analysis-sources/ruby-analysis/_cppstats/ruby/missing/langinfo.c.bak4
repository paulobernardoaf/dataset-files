

































#include "ruby/missing.h"
#include <stdlib.h>
#include <string.h>
#if defined _WIN32 || defined __CYGWIN__
#include <windows.h>
#if defined _WIN32 && !defined strncasecmp
#define strncasecmp strnicmp
#endif
#endif
#if defined(HAVE_LANGINFO_H)
#include "langinfo.h"
#endif

#define C_CODESET "US-ASCII" 



#if defined _WIN32 || defined __CYGWIN__
#define JA_CODESET "Windows-31J"
#else
#define JA_CODESET "EUC-JP"
#endif

#define digit(x) ((x) >= '0' && (x) <= '9')
#define strstart(s, n) (strncasecmp((s), (n), strlen(n)) == 0)

static char buf[16];

const char *
nl_langinfo_codeset(void)
{
const char *l, *p;
int n;

if (((l = getenv("LC_ALL")) && *l) ||
((l = getenv("LC_CTYPE")) && *l) ||
((l = getenv("LANG")) && *l)) {

if (!strcmp(l, "C") || !strcmp(l, "POSIX"))
return C_CODESET;

p = strchr(l, '.');
if (!p++) p = l;
if (strstart(p, "UTF"))
return "UTF-8";
if ((n = 5, strstart(p, "8859-")) || (n = 9, strstart(p, "ISO-8859-"))) {
if (digit(p[n])) {
p += n;
memcpy(buf, "ISO-8859-\0\0", 12);
buf[9] = *p++;
if (digit(*p)) buf[10] = *p++;
return buf;
}
}
if (strstart(p, "KOI8-R")) return "KOI8-R";
if (strstart(p, "KOI8-U")) return "KOI8-U";
if (strstart(p, "620")) return "TIS-620";
if (strstart(p, "2312")) return "GB2312";
if (strstart(p, "HKSCS")) return "Big5HKSCS"; 
if (strstart(p, "BIG5")) return "Big5";
if (strstart(p, "GBK")) return "GBK"; 
if (strstart(p, "18030")) return "GB18030"; 
if (strstart(p, "Shift_JIS") || strstart(p, "SJIS")) return "Windows-31J";

if (strstart(p, "euro")) return "ISO-8859-15";

if (strstart(l, "zh_TW")) return "Big5";
if (strstart(l, "zh_HK")) return "Big5HKSCS"; 
if (strstart(l, "zh")) return "GB2312";
if (strstart(l, "ja")) return JA_CODESET;
if (strstart(l, "ko")) return "EUC-KR";
if (strstart(l, "ru")) return "KOI8-R";
if (strstart(l, "uk")) return "KOI8-U";
if (strstart(l, "pl") || strstart(l, "hr") ||
strstart(l, "hu") || strstart(l, "cs") ||
strstart(l, "sk") || strstart(l, "sl")) return "ISO-8859-2";
if (strstart(l, "eo") || strstart(l, "mt")) return "ISO-8859-3";
if (strstart(l, "el")) return "ISO-8859-7";
if (strstart(l, "he")) return "ISO-8859-8";
if (strstart(l, "tr")) return "ISO-8859-9";
if (strstart(l, "th")) return "TIS-620"; 
if (strstart(l, "lt")) return "ISO-8859-13";
if (strstart(l, "cy")) return "ISO-8859-14";
if (strstart(l, "ro")) return "ISO-8859-2"; 
if (strstart(l, "am") || strstart(l, "vi")) return "UTF-8";



}
return NULL;
}

#if defined(HAVE_LANGINFO_H)
char *nl_langinfo(nl_item item)
{
const char *codeset;
if (item != CODESET)
return NULL;
codeset = nl_langinfo_codeset();
if (!codeset) codeset = C_CODESET;
return (char *)codeset;
}
#endif



#if defined(TEST)
#include <stdio.h>
int main()
{
printf("%s\n", nl_langinfo(CODESET));
return 0;
}
#endif
