




















#include "curlcheck.h"

#include "urldata.h"
#include "url.h"

#include "memdebug.h" 

static CURLcode unit_setup(void)
{
return CURLE_OK;
}

static void unit_stop(void)
{
}

#if defined(__MINGW32__) || (!defined(HAVE_FSETXATTR) && (!defined(__FreeBSD_version) || (__FreeBSD_version < 500000)))


UNITTEST_START
{
return 0;
}
UNITTEST_STOP
#else

bool stripcredentials(char **url);

struct checkthis {
const char *input;
const char *output;
};

static struct checkthis tests[] = {
{ "ninja://foo@example.com", "ninja://foo@example.com" },
{ "https://foo@example.com", "https://example.com/" },
{ "https://localhost:45", "https://localhost:45/" },
{ "https://foo@localhost:45", "https://localhost:45/" },
{ "http://daniel:password@localhost", "http://localhost/" },
{ "http://daniel@localhost", "http://localhost/" },
{ "http://localhost/", "http://localhost/" },
{ NULL, NULL } 
};

UNITTEST_START
{
bool cleanup;
char *url;
int i;
int rc = 0;

for(i = 0; tests[i].input; i++) {
url = (char *)tests[i].input;
cleanup = stripcredentials(&url);
printf("Test %u got input \"%s\", output: \"%s\"\n",
i, tests[i].input, url);

if(strcmp(tests[i].output, url)) {
fprintf(stderr, "Test %u got input \"%s\", expected output \"%s\"\n"
" Actual output: \"%s\"\n", i, tests[i].input, tests[i].output,
url);
rc++;
}
if(cleanup)
curl_free(url);
}
return rc;
}
UNITTEST_STOP
#endif
