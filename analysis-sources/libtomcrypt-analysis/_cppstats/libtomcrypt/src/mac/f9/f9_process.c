#include "tomcrypt_private.h"
#if defined(LTC_F9_MODE)
int f9_process(f9_state *f9, const unsigned char *in, unsigned long inlen)
{
int err, x;
LTC_ARGCHK(f9 != NULL);
LTC_ARGCHK(in != NULL);
if ((err = cipher_is_valid(f9->cipher)) != CRYPT_OK) {
return err;
}
if ((f9->blocksize > cipher_descriptor[f9->cipher].block_length) || (f9->blocksize < 0) ||
(f9->buflen > f9->blocksize) || (f9->buflen < 0)) {
return CRYPT_INVALID_ARG;
}
#if defined(LTC_FAST)
if (f9->buflen == 0) {
while (inlen >= (unsigned long)f9->blocksize) {
for (x = 0; x < f9->blocksize; x += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&(f9->IV[x]))) ^= *(LTC_FAST_TYPE_PTR_CAST(&(in[x])));
}
cipher_descriptor[f9->cipher].ecb_encrypt(f9->IV, f9->IV, &f9->key);
for (x = 0; x < f9->blocksize; x += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&(f9->ACC[x]))) ^= *(LTC_FAST_TYPE_PTR_CAST(&(f9->IV[x])));
}
in += f9->blocksize;
inlen -= f9->blocksize;
}
}
#endif
while (inlen) {
if (f9->buflen == f9->blocksize) {
cipher_descriptor[f9->cipher].ecb_encrypt(f9->IV, f9->IV, &f9->key);
for (x = 0; x < f9->blocksize; x++) {
f9->ACC[x] ^= f9->IV[x];
}
f9->buflen = 0;
}
f9->IV[f9->buflen++] ^= *in++;
--inlen;
}
return CRYPT_OK;
}
#endif
