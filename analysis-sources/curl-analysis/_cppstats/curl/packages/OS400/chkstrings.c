#include <stdlib.h>
#pragma enum(int)
#include "curl_setup.h"
#include "urldata.h"
#define EXPECTED_STRING_LASTZEROTERMINATED (STRING_DNS_LOCAL_IP6 + 1)
#define EXPECTED_STRING_LAST (STRING_COPYPOSTFIELDS + 1)
int main(int argc, char *argv[])
{
int rc = 0;
if (STRING_LASTZEROTERMINATED != EXPECTED_STRING_LASTZEROTERMINATED)
{
fprintf(stderr,"STRING_LASTZEROTERMINATED(%d) is not expected value(%d).\n",
STRING_LASTZEROTERMINATED, EXPECTED_STRING_LASTZEROTERMINATED);
rc += 1;
}
if (STRING_LAST != EXPECTED_STRING_LAST)
{
fprintf(stderr,"STRING_LAST(%d) is not expected value(%d).\n",
STRING_LAST, EXPECTED_STRING_LAST);
rc += 2;
}
if (rc != 0)
{
fprintf(stderr,"curl_easy_setopt_ccsid() in packages/OS400/ccsidcurl.c"
" may need updating if new strings are provided as input via the curl API.\n");
}
return rc;
}
