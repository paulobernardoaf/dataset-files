



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <string.h>












size_t strlcpy (char *tgt, const char *src, size_t bufsize)
{
size_t length = strlen(src);

if (bufsize > length)
memcpy(tgt, src, length + 1);
else
if (bufsize > 0)
memcpy(tgt, src, bufsize - 1), tgt[bufsize - 1] = '\0';

return length;
}
