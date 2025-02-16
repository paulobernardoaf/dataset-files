




















#include "curlcheck.h"

#include "dotdot.h"

#include "memdebug.h"

static CURLcode unit_setup(void)
{
return CURLE_OK;
}

static void unit_stop(void)
{

}

struct dotdot {
const char *input;
const char *output;
};

UNITTEST_START

unsigned int i;
int fails = 0;
const struct dotdot pairs[] = {
{ "/a/b/c/./../../g", "/a/g" },
{ "mid/content=5/../6", "mid/6" },
{ "/hello/../moo", "/moo" },
{ "/1/../1", "/1" },
{ "/1/./1", "/1/1" },
{ "/1/..", "/" },
{ "/1/.", "/1/" },
{ "/1/./..", "/" },
{ "/1/./../2", "/2" },
{ "/hello/1/./../2", "/hello/2" },
{ "test/this", "test/this" },
{ "test/this/../now", "test/now" },
{ "/1../moo../foo", "/1../moo../foo"},
{ "/../../moo", "/moo"},
{ "/../../moo?andnot/../yay", "/moo?andnot/../yay"},
{ "/123?foo=/./&bar=/../", "/123?foo=/./&bar=/../"},
{ "/../moo/..?what", "/?what" },
{ "/", "/" },
{ "", "" },
{ "/.../", "/.../" },
{ "./moo", "moo" },
{ "../moo", "moo" },
{ "/.", "/" },
{ "/..", "/" },
{ "/moo/..", "/" },
{ "..", "" },
{ ".", "" },
};

for(i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i++) {
char *out = Curl_dedotdotify(pairs[i].input);
abort_unless(out != NULL, "returned NULL!");

if(strcmp(out, pairs[i].output)) {
fprintf(stderr, "Test %u: '%s' gave '%s' instead of '%s'\n",
i, pairs[i].input, out, pairs[i].output);
fail("Test case output mismatched");
fails++;
}
else
fprintf(stderr, "Test %u: OK\n", i);
free(out);
}

fail_if(fails, "output mismatched");

UNITTEST_STOP
