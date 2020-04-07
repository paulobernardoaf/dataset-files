#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "r_regex.h"
#include "utils.h"
static char *regatoi(const RRegex*, char *, int);
static struct rerr {
int code;
char *name;
char *explain;
} rerrs[] = {
{ R_REGEX_NOMATCH, "R_REGEX_NOMATCH", "regexec() failed to match" },
{ R_REGEX_BADPAT, "R_REGEX_BADPAT", "invalid regular expression" },
{ R_REGEX_ECOLLATE, "R_REGEX_ECOLLATE", "invalid collating element" },
{ R_REGEX_ECTYPE, "R_REGEX_ECTYPE", "invalid character class" },
{ R_REGEX_EESCAPE, "R_REGEX_EESCAPE", "trailing backslash (\\)" },
{ R_REGEX_ESUBREG, "R_REGEX_ESUBREG", "invalid backreference number" },
{ R_REGEX_EBRACK, "R_REGEX_EBRACK", "brackets ([ ]) not balanced" },
{ R_REGEX_EPAREN, "R_REGEX_EPAREN", "parentheses not balanced" },
{ R_REGEX_EBRACE, "R_REGEX_EBRACE", "braces not balanced" },
{ R_REGEX_BADBR, "R_REGEX_BADBR", "invalid repetition count(s)" },
{ R_REGEX_ERANGE, "R_REGEX_ERANGE", "invalid character range" },
{ R_REGEX_ESPACE, "R_REGEX_ESPACE", "out of memory" },
{ R_REGEX_BADRPT, "R_REGEX_BADRPT", "repetition-operator operand invalid" },
{ R_REGEX_EMPTY, "R_REGEX_EMPTY", "empty (sub)expression" },
{ R_REGEX_ASSERT, "R_REGEX_ASSERT", "\"can't happen\" -- you found a bug" },
{ R_REGEX_INVARG, "R_REGEX_INVARG", "invalid argument to regex routine" },
{ 0, "", "*** unknown regexp error code ***" }
};
size_t
r_regex_error(int errcode, const RRegex *preg, char *errbuf, size_t errbuf_size)
{
struct rerr *r;
size_t len;
int target = errcode &~ R_REGEX_ITOA;
char *s;
char convbuf[50];
if (errcode == R_REGEX_ATOI) {
s = regatoi(preg, convbuf, sizeof convbuf);
} else {
for (r = rerrs; r->code != 0; r++) {
if (r->code == target) {
break;
}
}
if (errcode&R_REGEX_ITOA) {
if (r->code != 0) {
STRLCPY (convbuf, r->name, sizeof (convbuf)-1);
} else {
snprintf (convbuf, sizeof convbuf, "R_REGEX_0x%x", target);
}
s = convbuf;
} else {
s = r->explain;
}
}
len = strlen(s) + 1;
if (errbuf_size > 0) {
STRLCPY(errbuf, s, errbuf_size - 1);
}
return len;
}
static char *
regatoi(const RRegex *preg, char *localbuf, int localbufsize)
{
struct rerr *r;
for (r = rerrs; r->code != 0; r++) {
if (strcmp (r->name, preg->re_endp) == 0) {
break;
}
}
if (r->code == 0) {
return ("0");
}
(void)snprintf(localbuf, localbufsize, "%d", r->code);
return(localbuf);
}
