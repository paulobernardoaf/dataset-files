







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)








int x25519_import(const unsigned char *in, unsigned long inlen, curve25519_key *key)
{
int err;
unsigned long key_len;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);

key_len = sizeof(key->pub);
if ((err = x509_decode_subject_public_key_info(in, inlen, PKA_X25519, key->pub, &key_len, LTC_ASN1_EOL, NULL, 0uL)) == CRYPT_OK) {
key->type = PK_PUBLIC;
key->algo = PKA_X25519;
}
return err;
}

#endif




