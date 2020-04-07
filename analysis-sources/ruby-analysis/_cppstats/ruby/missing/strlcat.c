#include "ruby/missing.h"
#include <sys/types.h>
#include <string.h>
size_t
strlcat(char *dst, const char *src, size_t dsize)
{
const char *odst = dst;
const char *osrc = src;
size_t n = dsize;
size_t dlen;
while (n-- != 0 && *dst != '\0')
dst++;
dlen = dst - odst;
n = dsize - dlen;
if (n-- == 0)
return(dlen + strlen(src));
while (*src != '\0') {
if (n != 0) {
*dst++ = *src;
n--;
}
src++;
}
*dst = '\0';
return(dlen + (src - osrc)); 
}
