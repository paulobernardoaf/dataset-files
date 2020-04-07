#define CURL_NO_OLDIES
#include "curl_setup.h" 
#include <curl/curl.h> 
#if defined(macintosh) && defined(__MRC__)
#define main(x,y) curl_main(x,y)
#endif
#if defined(TPF)
#undef select
#define select(a,b,c,d,e) tpf_select_bsd(a,b,c,d,e)
#define CONF_DEFAULT (0|CONF_NOPROGRESS)
#endif
#if !defined(OS)
#define OS "unknown"
#endif
#if !defined(UNPRINTABLE_CHAR)
#define UNPRINTABLE_CHAR '.'
#endif
#if !defined(HAVE_STRDUP)
#include "tool_strdup.h"
#endif
