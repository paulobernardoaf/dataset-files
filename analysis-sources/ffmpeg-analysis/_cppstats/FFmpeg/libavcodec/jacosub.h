#include "libavutil/common.h"
#define JSS_MAX_LINESIZE 512
static av_always_inline int jss_whitespace(char c)
{
return c == ' ' || (c >= '\t' && c <= '\r');
}
static av_always_inline const char *jss_skip_whitespace(const char *p)
{
while (jss_whitespace(*p))
p++;
return p;
}
