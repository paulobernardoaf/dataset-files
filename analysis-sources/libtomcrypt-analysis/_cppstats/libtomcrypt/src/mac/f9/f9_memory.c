#include "tomcrypt_private.h"
#if defined(LTC_F9_MODE)
int f9_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
f9_state *f9;
int err;
if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
return err;
}
if (cipher_descriptor[cipher].f9_memory != NULL) {
return cipher_descriptor[cipher].f9_memory(key, keylen, in, inlen, out, outlen);
}
f9 = XCALLOC(1, sizeof(*f9));
if (f9 == NULL) {
return CRYPT_MEM;
}
if ((err = f9_init(f9, cipher, key, keylen)) != CRYPT_OK) {
goto done;
}
if ((err = f9_process(f9, in, inlen)) != CRYPT_OK) {
goto done;
}
err = f9_done(f9, out, outlen);
done:
XFREE(f9);
return err;
}
#endif
