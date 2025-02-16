#include "tomcrypt_private.h"
#if defined(LTC_CHACHA20POLY1305_MODE)
int chacha20poly1305_done(chacha20poly1305_state *st, unsigned char *tag, unsigned long *taglen)
{
unsigned char padzero[16] = { 0 };
unsigned long padlen;
unsigned char buf[16];
int err;
LTC_ARGCHK(st != NULL);
padlen = 16 - (unsigned long)(st->ctlen % 16);
if (padlen < 16) {
if ((err = poly1305_process(&st->poly, padzero, padlen)) != CRYPT_OK) return err;
}
STORE64L(st->aadlen, buf);
STORE64L(st->ctlen, buf + 8);
if ((err = poly1305_process(&st->poly, buf, 16)) != CRYPT_OK) return err;
if ((err = poly1305_done(&st->poly, tag, taglen)) != CRYPT_OK) return err;
if ((err = chacha_done(&st->chacha)) != CRYPT_OK) return err;
return CRYPT_OK;
}
#endif
