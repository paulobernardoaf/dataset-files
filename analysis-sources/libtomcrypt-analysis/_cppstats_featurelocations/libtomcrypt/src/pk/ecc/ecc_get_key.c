








#include "tomcrypt_private.h"

#if defined(LTC_MECC)









int ecc_get_key(unsigned char *out, unsigned long *outlen, int type, const ecc_key *key)
{
unsigned long size, ksize;
int err, compressed;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);

size = key->dp.size;
compressed = type & PK_COMPRESSED ? 1 : 0;
type &= ~PK_COMPRESSED;

if (type == PK_PUBLIC) {
if ((err = ltc_ecc_export_point(out, outlen, key->pubkey.x, key->pubkey.y, size, compressed)) != CRYPT_OK) {
return err;
}
}
else if (type == PK_PRIVATE) {
if (key->type != PK_PRIVATE) return CRYPT_PK_TYPE_MISMATCH;
*outlen = size;
if (size > *outlen) return CRYPT_BUFFER_OVERFLOW;
if ((ksize = mp_unsigned_bin_size(key->k)) > size) return CRYPT_BUFFER_OVERFLOW;

if ((err = mp_to_unsigned_bin(key->k, out + (size - ksize))) != CRYPT_OK) return err;
zeromem(out, size - ksize);
}
else {
return CRYPT_INVALID_ARG;
}

return CRYPT_OK;
}

#endif




