#include "tomcrypt_private.h"
#if defined(LTC_PELICAN)
int pelican_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out)
{
pelican_state *pel;
int err;
pel = XMALLOC(sizeof(*pel));
if (pel == NULL) {
return CRYPT_MEM;
}
if ((err = pelican_init(pel, key, keylen)) != CRYPT_OK) {
XFREE(pel);
return err;
}
if ((err = pelican_process(pel, in ,inlen)) != CRYPT_OK) {
XFREE(pel);
return err;
}
err = pelican_done(pel, out);
XFREE(pel);
return err;
}
#endif
