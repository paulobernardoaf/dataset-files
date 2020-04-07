#include <limits.h>
#include <stdlib.h>
#include "libavutil/avstring.h"
#include "libavutil/mathematics.h"
static const char *check_nan_suffix(const char *s)
{
const char *start = s;
if (*s++ != '(')
return start;
while ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') ||
(*s >= '0' && *s <= '9') || *s == '_')
s++;
return *s == ')' ? s + 1 : start;
}
#undef strtod
double strtod(const char *, char **);
double avpriv_strtod(const char *nptr, char **endptr)
{
const char *end;
double res;
while (av_isspace(*nptr))
nptr++;
if (!av_strncasecmp(nptr, "infinity", 8)) {
end = nptr + 8;
res = INFINITY;
} else if (!av_strncasecmp(nptr, "inf", 3)) {
end = nptr + 3;
res = INFINITY;
} else if (!av_strncasecmp(nptr, "+infinity", 9)) {
end = nptr + 9;
res = INFINITY;
} else if (!av_strncasecmp(nptr, "+inf", 4)) {
end = nptr + 4;
res = INFINITY;
} else if (!av_strncasecmp(nptr, "-infinity", 9)) {
end = nptr + 9;
res = -INFINITY;
} else if (!av_strncasecmp(nptr, "-inf", 4)) {
end = nptr + 4;
res = -INFINITY;
} else if (!av_strncasecmp(nptr, "nan", 3)) {
end = check_nan_suffix(nptr + 3);
res = NAN;
} else if (!av_strncasecmp(nptr, "+nan", 4) ||
!av_strncasecmp(nptr, "-nan", 4)) {
end = check_nan_suffix(nptr + 4);
res = NAN;
} else if (!av_strncasecmp(nptr, "0x", 2) ||
!av_strncasecmp(nptr, "-0x", 3) ||
!av_strncasecmp(nptr, "+0x", 3)) {
res = strtoll(nptr, (char **)&end, 16);
} else {
res = strtod(nptr, (char **)&end);
}
if (endptr)
*endptr = (char *)end;
return res;
}
