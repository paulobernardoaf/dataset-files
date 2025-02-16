








#include "tomcrypt_private.h"

#if defined(LTC_CHACHA20POLY1305_MODE)









int chacha20poly1305_encrypt(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out)
{
unsigned char padzero[16] = { 0 };
unsigned long padlen;
int err;

LTC_ARGCHK(st != NULL);

if ((err = chacha_crypt(&st->chacha, in, inlen, out)) != CRYPT_OK) return err;
if (st->aadflg) {
padlen = 16 - (unsigned long)(st->aadlen % 16);
if (padlen < 16) {
if ((err = poly1305_process(&st->poly, padzero, padlen)) != CRYPT_OK) return err;
}
st->aadflg = 0; 
}
if ((err = poly1305_process(&st->poly, out, inlen)) != CRYPT_OK) return err;
st->ctlen += (ulong64)inlen;
return CRYPT_OK;
}

#endif




