#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_set_key(const unsigned char *in, unsigned long inlen, int type, ecc_key *key)
{
int err;
void *prime, *a, *b;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen > 0);
prime = key->dp.prime;
a = key->dp.A;
b = key->dp.B;
if (type == PK_PRIVATE) {
if ((err = mp_read_unsigned_bin(key->k, (unsigned char *)in, inlen)) != CRYPT_OK) {
goto error;
}
if (mp_iszero(key->k) || (mp_cmp(key->k, key->dp.order) != LTC_MP_LT)) {
err = CRYPT_INVALID_PACKET;
goto error;
}
if ((err = ltc_mp.ecc_ptmul(key->k, &key->dp.base, &key->pubkey, a, prime, 1)) != CRYPT_OK) { goto error; }
}
else if (type == PK_PUBLIC) {
if ((err = ltc_ecc_import_point(in, inlen, prime, a, b, key->pubkey.x, key->pubkey.y)) != CRYPT_OK) { goto error; }
if ((err = mp_set(key->pubkey.z, 1)) != CRYPT_OK) { goto error; }
}
else {
err = CRYPT_INVALID_PACKET;
goto error;
}
if ((err = ltc_ecc_verify_key(key)) != CRYPT_OK) {
goto error;
}
key->type = type;
return CRYPT_OK;
error:
ecc_free(key);
return err;
}
#endif
