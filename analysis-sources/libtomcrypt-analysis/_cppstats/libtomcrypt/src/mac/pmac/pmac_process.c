#include "tomcrypt_private.h"
#if defined(LTC_PMAC)
int pmac_process(pmac_state *pmac, const unsigned char *in, unsigned long inlen)
{
int err, n;
unsigned long x;
unsigned char Z[MAXBLOCKSIZE];
LTC_ARGCHK(pmac != NULL);
LTC_ARGCHK(in != NULL);
if ((err = cipher_is_valid(pmac->cipher_idx)) != CRYPT_OK) {
return err;
}
if ((pmac->buflen > (int)sizeof(pmac->block)) || (pmac->buflen < 0) ||
(pmac->block_len > (int)sizeof(pmac->block)) || (pmac->buflen > pmac->block_len)) {
return CRYPT_INVALID_ARG;
}
#if defined(LTC_FAST)
if (pmac->buflen == 0 && inlen > 16) {
unsigned long y;
for (x = 0; x < (inlen - 16); x += 16) {
pmac_shift_xor(pmac);
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&Z[y])) = *(LTC_FAST_TYPE_PTR_CAST(&in[y])) ^ *(LTC_FAST_TYPE_PTR_CAST(&pmac->Li[y]));
}
if ((err = cipher_descriptor[pmac->cipher_idx].ecb_encrypt(Z, Z, &pmac->key)) != CRYPT_OK) {
return err;
}
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&pmac->checksum[y])) ^= *(LTC_FAST_TYPE_PTR_CAST(&Z[y]));
}
in += 16;
}
inlen -= x;
}
#endif
while (inlen != 0) {
if (pmac->buflen == pmac->block_len) {
pmac_shift_xor(pmac);
for (x = 0; x < (unsigned long)pmac->block_len; x++) {
Z[x] = pmac->Li[x] ^ pmac->block[x];
}
if ((err = cipher_descriptor[pmac->cipher_idx].ecb_encrypt(Z, Z, &pmac->key)) != CRYPT_OK) {
return err;
}
for (x = 0; x < (unsigned long)pmac->block_len; x++) {
pmac->checksum[x] ^= Z[x];
}
pmac->buflen = 0;
}
n = MIN(inlen, (unsigned long)(pmac->block_len - pmac->buflen));
XMEMCPY(pmac->block + pmac->buflen, in, n);
pmac->buflen += n;
inlen -= n;
in += n;
}
#if defined(LTC_CLEAN_STACK)
zeromem(Z, sizeof(Z));
#endif
return CRYPT_OK;
}
#endif
