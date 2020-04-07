








#include "tomcrypt_private.h"

#if defined(LTC_MDH)








int dh_import(const unsigned char *in, unsigned long inlen, dh_key *key)
{
unsigned char flags[1];
int err;
unsigned long version;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);


if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
return err;
}


err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &version,
LTC_ASN1_BIT_STRING, 1UL, &flags,
LTC_ASN1_EOL, 0UL, NULL);
if (err != CRYPT_OK && err != CRYPT_INPUT_TOO_LONG) {
goto error;
}

if (version == 0) {
if (flags[0] == 1) {
key->type = PK_PRIVATE;
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &version,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->prime,
LTC_ASN1_INTEGER, 1UL, key->base,
LTC_ASN1_INTEGER, 1UL, key->x,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto error;
}

if ((err = mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK) {
goto error;
}
}
else if (flags[0] == 0) {
key->type = PK_PUBLIC;
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &version,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->prime,
LTC_ASN1_INTEGER, 1UL, key->base,
LTC_ASN1_INTEGER, 1UL, key->y,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto error;
}
}
else {
err = CRYPT_INVALID_PACKET;
goto error;
}
}
else {
err = CRYPT_INVALID_PACKET;
goto error;
}


if ((err = dh_check_pubkey(key)) != CRYPT_OK) {
goto error;
}

return CRYPT_OK;

error:
dh_free(key);
return err;
}

#endif 




