#include "curl_setup.h"
#include <stdio.h>
static const char *disabled[]={
#if defined(CURL_DISABLE_COOKIES)
"cookies",
#endif
#if defined(CURL_DISABLE_CRYPTO_AUTH)
"crypto",
#endif
#if defined(CURL_DISABLE_DOH)
"DoH",
#endif
#if defined(CURL_DISABLE_HTTP_AUTH)
"HTTP-auth",
#endif
#if defined(CURL_DISABLE_MIME)
"Mime",
#endif
#if defined(CURL_DISABLE_NETRC)
"netrc",
#endif
#if defined(CURL_DISABLE_PARSEDATE)
"parsedate",
#endif
#if defined(CURL_DISABLE_PROXY)
"proxy",
#endif
#if defined(CURL_DISABLE_SHUFFLE_DNS)
"shuffle-dns",
#endif
#if defined(CURL_DISABLE_TYPECHECK)
"typecheck",
#endif
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
"verbose-strings",
#endif
NULL
};
int main(void)
{
int i;
for(i = 0; disabled[i]; i++)
printf("%s\n", disabled[i]);
return 0;
}
