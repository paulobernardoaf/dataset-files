#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#undef NDEBUG
#include <assert.h>
#include <vlc_common.h>
#include "../input/mrl_helpers.h"
#define MAX_RESULT 10
static const struct {
const char *payload;
const char *results[MAX_RESULT];
const char *extra;
bool success;
} testcase[] = {
{ "!/hello.zip!/goodbye.rar",
{ "hello.zip", "goodbye.rar" }, NULL, true },
{ "!/hello.zip!/goodbye.rar?t=0&s=0",
{ "hello.zip", "goodbye.rar" }, "t=0&s=0", true },
{ "!/hello.zip!/goodbye.rar?",
{ "hello.zip", "goodbye.rar" }, "", true },
{ "!/he%20%25""llo.zip!/good%2520bye.rar",
{ "he %llo.zip", "good%20bye.rar" }, NULL, true },
{ "",
{}, NULL, true },
{ "?extra",
{}, "?extra", true },
{ "!/he!llo.zip!/goodbye.rar",
{}, NULL, false },
{ "!/hello.zip!/!",
{}, NULL, false },
};
int main (void)
{
for (size_t i = 0; i < ARRAY_SIZE(testcase); ++i)
{
vlc_array_t out;
const char *extra = NULL;
int ret = mrl_FragmentSplit(&out, &extra, testcase[i].payload);
if (testcase[i].success)
{
assert(ret == VLC_SUCCESS);
if (extra != NULL)
assert(strcmp(extra, testcase[i].extra) == 0);
else
assert(testcase[i].extra == NULL);
const char *p = testcase[i].payload + 2;
for (size_t j = 0; testcase[i].results[j] != NULL; ++j)
{
assert(j < vlc_array_count(&out) && j < MAX_RESULT);
char *res = vlc_array_item_at_index(&out, j);
assert(strcmp(testcase[i].results[j], res) == 0);
char *res_escaped = NULL;
ret = mrl_EscapeFragmentIdentifier(&res_escaped, res);
assert(ret == VLC_SUCCESS && res_escaped != NULL);
assert(strncmp(p, res_escaped, strlen(res_escaped)) == 0);
p += strlen(res_escaped) + 2;
free(res_escaped);
free(res);
}
vlc_array_clear(&out);
}
else
{
assert(ret != VLC_SUCCESS);
}
}
return 0;
}
