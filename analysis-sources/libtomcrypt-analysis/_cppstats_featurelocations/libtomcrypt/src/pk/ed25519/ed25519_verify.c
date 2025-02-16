







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)










int ed25519_verify(const unsigned char *msg, unsigned long msglen,
const unsigned char *sig, unsigned long siglen,
int *stat, const curve25519_key *public_key)
{
unsigned char* m;
unsigned long long mlen;
int err;

LTC_ARGCHK(msg != NULL);
LTC_ARGCHK(sig != NULL);
LTC_ARGCHK(stat != NULL);
LTC_ARGCHK(public_key != NULL);

*stat = 0;

if (siglen != 64uL) return CRYPT_INVALID_ARG;
if (public_key->algo != PKA_ED25519) return CRYPT_PK_INVALID_TYPE;

mlen = msglen + siglen;
if ((mlen < msglen) || (mlen < siglen)) return CRYPT_OVERFLOW;

m = XMALLOC(mlen);
if (m == NULL) return CRYPT_MEM;

XMEMCPY(m, sig, siglen);
XMEMCPY(m + siglen, msg, msglen);

err = tweetnacl_crypto_sign_open(stat,
m, &mlen,
m, mlen,
public_key->pub);

#if defined(LTC_CLEAN_STACK)
zeromem(m, mlen);
#endif
XFREE(m);

return err;
}

#endif




