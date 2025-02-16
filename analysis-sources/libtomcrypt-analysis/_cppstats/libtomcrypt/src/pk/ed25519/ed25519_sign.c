#include "tomcrypt_private.h"
#if defined(LTC_CURVE25519)
int ed25519_sign(const unsigned char *msg, unsigned long msglen,
unsigned char *sig, unsigned long *siglen,
const curve25519_key *private_key)
{
unsigned char *s;
unsigned long long smlen;
int err;
LTC_ARGCHK(msg != NULL);
LTC_ARGCHK(sig != NULL);
LTC_ARGCHK(siglen != NULL);
LTC_ARGCHK(private_key != NULL);
if (private_key->algo != PKA_ED25519) return CRYPT_PK_INVALID_TYPE;
if (private_key->type != PK_PRIVATE) return CRYPT_PK_INVALID_TYPE;
if (*siglen < 64uL) {
*siglen = 64uL;
return CRYPT_BUFFER_OVERFLOW;
}
smlen = msglen + 64;
s = XMALLOC(smlen);
if (s == NULL) return CRYPT_MEM;
err = tweetnacl_crypto_sign(s, &smlen,
msg, msglen,
private_key->priv, private_key->pub);
XMEMCPY(sig, s, 64uL);
*siglen = 64uL;
#if defined(LTC_CLEAN_STACK)
zeromem(s, smlen);
#endif
XFREE(s);
return err;
}
#endif
