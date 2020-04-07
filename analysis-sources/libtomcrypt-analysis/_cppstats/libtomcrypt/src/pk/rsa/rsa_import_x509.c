#include "tomcrypt_private.h"
#if defined(LTC_MRSA)
static int _rsa_decode(const unsigned char *in, unsigned long inlen, rsa_key *key)
{
return der_decode_sequence_multi(in, inlen,
LTC_ASN1_INTEGER, 1UL, key->N,
LTC_ASN1_INTEGER, 1UL, key->e,
LTC_ASN1_EOL, 0UL, NULL);
}
int rsa_import_x509(const unsigned char *in, unsigned long inlen, rsa_key *key)
{
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
if ((err = rsa_init(key)) != CRYPT_OK) {
return err;
}
if ((err = x509_decode_public_key_from_certificate(in, inlen,
PKA_RSA, LTC_ASN1_NULL,
NULL, NULL,
(public_key_decode_cb)_rsa_decode, key)) != CRYPT_OK) {
rsa_free(key);
} else {
key->type = PK_PUBLIC;
}
return err;
}
#endif 
