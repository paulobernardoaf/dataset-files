







#include "tomcrypt_private.h"






#if defined(LTC_F9_MODE)








int f9_init(f9_state *f9, int cipher, const unsigned char *key, unsigned long keylen)
{
int x, err;

LTC_ARGCHK(f9 != NULL);
LTC_ARGCHK(key != NULL);


if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
return err;
}

#if defined(LTC_FAST)
if (cipher_descriptor[cipher].block_length % sizeof(LTC_FAST_TYPE)) {
return CRYPT_INVALID_ARG;
}
#endif

if ((err = cipher_descriptor[cipher].setup(key, keylen, 0, &f9->key)) != CRYPT_OK) {
goto done;
}


for (x = 0; (unsigned)x < keylen; x++) {
f9->akey[x] = key[x] ^ 0xAA;
}


zeromem(f9->IV, cipher_descriptor[cipher].block_length);
zeromem(f9->ACC, cipher_descriptor[cipher].block_length);
f9->blocksize = cipher_descriptor[cipher].block_length;
f9->cipher = cipher;
f9->buflen = 0;
f9->keylen = keylen;
done:
return err;
}

#endif





