#include <stdio.h>
#include "libavutil/common.h"
#include "libavutil/mem.h"
#include "libavutil/avstring.h"
int main(void)
{
int i;
char *fullpath, *ptr;
static const char * const strings[] = {
"''",
"",
":",
"\\",
"'",
" '' :",
" '' '' :",
"foo '' :",
"'foo'",
"foo ",
" ' foo ' ",
"foo\\",
"foo': blah:blah",
"foo\\: blah:blah",
"foo\'",
"'foo : ' :blahblah",
"\\ :blah",
" foo",
" foo ",
" foo \\ ",
"foo ':blah",
" foo bar : blahblah",
"\\f\\o\\o",
"'foo : \\ \\ ' : blahblah",
"'\\fo\\o:': blahblah",
"\\'fo\\o\\:': foo ' :blahblah"
};
const char *haystack = "Education consists mainly in what we have unlearned.";
const char * const needle[] = {"learned.", "unlearned.", "Unlearned"};
printf("Testing av_get_token()\n");
for (i = 0; i < FF_ARRAY_ELEMS(strings); i++) {
const char *p = strings[i];
char *q;
printf("|%s|", p);
q = av_get_token(&p, ":");
printf(" -> |%s|", q);
printf(" + |%s|\n", p);
av_free(q);
}
printf("Testing av_append_path_component()\n");
#define TEST_APPEND_PATH_COMPONENT(path, component, expected) fullpath = av_append_path_component((path), (component)); printf("%s = %s\n", fullpath ? fullpath : "(null)", expected); av_free(fullpath);
TEST_APPEND_PATH_COMPONENT(NULL, NULL, "(null)")
TEST_APPEND_PATH_COMPONENT("path", NULL, "path");
TEST_APPEND_PATH_COMPONENT(NULL, "comp", "comp");
TEST_APPEND_PATH_COMPONENT("path", "comp", "path/comp");
TEST_APPEND_PATH_COMPONENT("path/", "comp", "path/comp");
TEST_APPEND_PATH_COMPONENT("path", "/comp", "path/comp");
TEST_APPEND_PATH_COMPONENT("path/", "/comp", "path/comp");
TEST_APPEND_PATH_COMPONENT("path/path2/", "/comp/comp2", "path/path2/comp/comp2");
#define TEST_STRNSTR(haystack, needle, hay_length, expected) ptr = av_strnstr(haystack, needle, hay_length); if (ptr != expected){ printf("expected: %p, received %p\n", expected, ptr); }
TEST_STRNSTR(haystack, needle [0], strlen(haystack), haystack+44);
TEST_STRNSTR(haystack, needle [1], strlen(haystack), haystack+42);
TEST_STRNSTR(haystack, needle [2], strlen(haystack), NULL );
TEST_STRNSTR(haystack, strings[1], strlen(haystack), haystack );
#define TEST_STRIREPLACE(haystack, needle, expected) ptr = av_strireplace(haystack, needle, "instead"); if (ptr == NULL) { printf("error, received null pointer!\n"); } else { if (strcmp(ptr, expected) != 0) printf( "expected: %s, received: %s\n", expected, ptr); av_free(ptr); }
TEST_STRIREPLACE(haystack, needle [0], "Education consists mainly in what we have uninstead");
TEST_STRIREPLACE(haystack, needle [1], "Education consists mainly in what we have instead");
TEST_STRIREPLACE(haystack, needle [2], "Education consists mainly in what we have instead.");
TEST_STRIREPLACE(haystack, needle [1], "Education consists mainly in what we have instead");
#define TEST_D2STR(value, expected) if((ptr = av_d2str(value)) == NULL){ printf("error, received null pointer!\n"); } else { if(strcmp(ptr, expected) != 0) printf( "expected: %s, received: %s\n", expected, ptr); av_free(ptr); }
TEST_D2STR(0 , "0.000000");
TEST_D2STR(-1.2333234, "-1.233323");
TEST_D2STR(-1.2333237, "-1.233324");
return 0;
}
