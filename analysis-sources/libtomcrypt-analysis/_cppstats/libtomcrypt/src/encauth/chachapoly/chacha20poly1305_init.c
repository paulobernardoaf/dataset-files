#include "tomcrypt_private.h"
#if defined(LTC_CHACHA20POLY1305_MODE)
int chacha20poly1305_init(chacha20poly1305_state *st, const unsigned char *key, unsigned long keylen)
{
return chacha_setup(&st->chacha, key, keylen, 20);
}
#endif
