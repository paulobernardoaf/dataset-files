







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)

static int _x25519_decode(const unsigned char *in, unsigned long inlen, curve25519_key *key)
{
if (inlen != sizeof(key->pub)) return CRYPT_PK_INVALID_SIZE;
XMEMCPY(key->pub, in, sizeof(key->pub));
return CRYPT_OK;
}








int x25519_import_x509(const unsigned char *in, unsigned long inlen, curve25519_key *key)
{
int err;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);

if ((err = x509_decode_public_key_from_certificate(in, inlen,
PKA_X25519,
LTC_ASN1_EOL, NULL, NULL,
(public_key_decode_cb)_x25519_decode, key)) != CRYPT_OK) {
return err;
}
key->type = PK_PUBLIC;
key->algo = PKA_X25519;

return err;
}

#endif




