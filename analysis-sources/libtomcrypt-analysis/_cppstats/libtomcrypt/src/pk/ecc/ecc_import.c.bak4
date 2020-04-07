








#include "tomcrypt_private.h"






#if defined(LTC_MECC)








int ecc_import(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
return ecc_import_ex(in, inlen, key, NULL);
}









int ecc_import_ex(const unsigned char *in, unsigned long inlen, ecc_key *key, const ltc_ecc_curve *cu)
{
unsigned long key_size;
unsigned char flags[1];
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);


err = der_decode_sequence_multi(in, inlen, LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_SHORT_INTEGER, 1UL, &key_size,
LTC_ASN1_EOL, 0UL, NULL);
if (err != CRYPT_OK && err != CRYPT_INPUT_TOO_LONG) {
return err;
}


if (cu == NULL) {
if ((err = ecc_set_curve_by_size(key_size, key)) != CRYPT_OK) { goto done; }
} else {
if ((err = ecc_set_curve(cu, key)) != CRYPT_OK) { goto done; }
}

if (flags[0] == 1) {

key->type = PK_PRIVATE;
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_SHORT_INTEGER, 1UL, &key_size,
LTC_ASN1_INTEGER, 1UL, key->pubkey.x,
LTC_ASN1_INTEGER, 1UL, key->pubkey.y,
LTC_ASN1_INTEGER, 1UL, key->k,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto done;
}
} else if (flags[0] == 0) {

key->type = PK_PUBLIC;
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_SHORT_INTEGER, 1UL, &key_size,
LTC_ASN1_INTEGER, 1UL, key->pubkey.x,
LTC_ASN1_INTEGER, 1UL, key->pubkey.y,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto done;
}
}
else {
err = CRYPT_INVALID_PACKET;
goto done;
}


if ((err = mp_set(key->pubkey.z, 1)) != CRYPT_OK) { goto done; }


if ((err = ltc_ecc_verify_key(key)) != CRYPT_OK) { goto done; }


return CRYPT_OK;

done:
ecc_free(key);
return err;
}
#endif




