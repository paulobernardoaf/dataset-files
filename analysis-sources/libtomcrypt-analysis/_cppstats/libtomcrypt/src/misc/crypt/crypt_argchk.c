#include "tomcrypt_private.h"
#if (ARGTYPE == 0)
void crypt_argchk(const char *v, const char *s, int d)
{
fprintf(stderr, "LTC_ARGCHK '%s' failure on line %d of file %s\n",
v, d, s);
abort();
}
#endif
