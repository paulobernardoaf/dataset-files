#include "curlcheck.h"
#include "tool_getparam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memdebug.h" 
static CURLcode unit_setup(void)
{
return CURLE_OK;
}
static void unit_stop(void)
{
}
UNITTEST_START
const char *values[] = {
"foo:bar:baz", "foo", "bar:baz",
"foo\\:bar:baz", "foo:bar", "baz",
"foo\\\\:bar:baz", "foo\\", "bar:baz",
"foo:bar\\:baz", "foo", "bar\\:baz",
"foo:bar\\\\:baz", "foo", "bar\\\\:baz",
"foo\\bar\\baz", "foo\\bar\\baz", NULL,
"foo\\\\bar\\\\baz", "foo\\bar\\baz", NULL,
"foo\\", "foo\\", NULL,
"foo\\\\", "foo\\", NULL,
"foo:bar\\", "foo", "bar\\",
"foo:bar\\\\", "foo", "bar\\\\",
"foo:bar:", "foo", "bar:",
"foo\\::bar\\:", "foo:", "bar\\:",
"pkcs11:foobar", "pkcs11:foobar", NULL,
"PKCS11:foobar", "PKCS11:foobar", NULL,
"PkCs11:foobar", "PkCs11:foobar", NULL,
#if defined(WIN32)
"c:\\foo:bar:baz", "c:\\foo", "bar:baz",
"c:\\foo\\:bar:baz", "c:\\foo:bar", "baz",
"c:\\foo\\\\:bar:baz", "c:\\foo\\", "bar:baz",
"c:\\foo:bar\\:baz", "c:\\foo", "bar\\:baz",
"c:\\foo:bar\\\\:baz", "c:\\foo", "bar\\\\:baz",
"c:\\foo\\bar\\baz", "c:\\foo\\bar\\baz", NULL,
"c:\\foo\\\\bar\\\\baz", "c:\\foo\\bar\\baz", NULL,
"c:\\foo\\", "c:\\foo\\", NULL,
"c:\\foo\\\\", "c:\\foo\\", NULL,
"c:\\foo:bar\\", "c:\\foo", "bar\\",
"c:\\foo:bar\\\\", "c:\\foo", "bar\\\\",
"c:\\foo:bar:", "c:\\foo", "bar:",
"c:\\foo\\::bar\\:", "c:\\foo:", "bar\\:",
#endif
NULL, NULL, NULL,
};
const char **p;
char *certname, *passphrase;
for(p = values; *p; p += 3) {
parse_cert_parameter(p[0], &certname, &passphrase);
if(p[1]) {
if(certname) {
if(strcmp(p[1], certname)) {
printf("expected certname '%s' but got '%s' "
"for -E param '%s'\n", p[1], certname, p[0]);
fail("assertion failure");
}
}
else {
printf("expected certname '%s' but got NULL "
"for -E param '%s'\n", p[1], p[0]);
fail("assertion failure");
}
}
else {
if(certname) {
printf("expected certname NULL but got '%s' "
"for -E param '%s'\n", certname, p[0]);
fail("assertion failure");
}
}
if(p[2]) {
if(passphrase) {
if(strcmp(p[2], passphrase)) {
printf("expected passphrase '%s' but got '%s'"
"for -E param '%s'\n", p[2], passphrase, p[0]);
fail("assertion failure");
}
}
else {
printf("expected passphrase '%s' but got NULL "
"for -E param '%s'\n", p[2], p[0]);
fail("assertion failure");
}
}
else {
if(passphrase) {
printf("expected passphrase NULL but got '%s' "
"for -E param '%s'\n", passphrase, p[0]);
fail("assertion failure");
}
}
if(certname)
free(certname);
if(passphrase)
free(passphrase);
}
UNITTEST_STOP
