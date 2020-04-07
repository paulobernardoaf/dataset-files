







#include "tomcrypt_private.h"






#if defined(LTC_F9_MODE)







int f9_done(f9_state *f9, unsigned char *out, unsigned long *outlen)
{
int err, x;
LTC_ARGCHK(f9 != NULL);
LTC_ARGCHK(out != NULL);


if ((err = cipher_is_valid(f9->cipher)) != CRYPT_OK) {
return err;
}

if ((f9->blocksize > cipher_descriptor[f9->cipher].block_length) || (f9->blocksize < 0) ||
(f9->buflen > f9->blocksize) || (f9->buflen < 0)) {
return CRYPT_INVALID_ARG;
}

if (f9->buflen != 0) {

cipher_descriptor[f9->cipher].ecb_encrypt(f9->IV, f9->IV, &f9->key);
f9->buflen = 0;
for (x = 0; x < f9->blocksize; x++) {
f9->ACC[x] ^= f9->IV[x];
}
}


if ((err = cipher_descriptor[f9->cipher].setup(f9->akey, f9->keylen, 0, &f9->key)) != CRYPT_OK) {
return err;
}


cipher_descriptor[f9->cipher].ecb_encrypt(f9->ACC, f9->ACC, &f9->key);
cipher_descriptor[f9->cipher].done(&f9->key);


for (x = 0; x < f9->blocksize && (unsigned long)x < *outlen; x++) {
out[x] = f9->ACC[x];
}
*outlen = x;

#if defined(LTC_CLEAN_STACK)
zeromem(f9, sizeof(*f9));
#endif
return CRYPT_OK;
}

#endif





