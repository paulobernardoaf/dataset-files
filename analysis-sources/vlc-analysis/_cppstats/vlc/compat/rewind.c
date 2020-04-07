#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdio.h>
void rewind (FILE *stream)
{
fseek (stream, 0L, SEEK_SET);
clearerr (stream);
}
