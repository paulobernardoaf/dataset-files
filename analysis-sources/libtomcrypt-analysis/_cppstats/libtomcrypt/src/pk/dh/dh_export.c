#include "tomcrypt_private.h"
#if defined(LTC_MDH)
int dh_export(unsigned char *out, unsigned long *outlen, int type, const dh_key *key)
{
unsigned char flags[1];
int err;
unsigned long version = 0;
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
LTC_ARGCHK(key != NULL);
if (type == PK_PRIVATE) {
flags[0] = 1;
err = der_encode_sequence_multi(out, outlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &version,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->prime,
LTC_ASN1_INTEGER, 1UL, key->base,
LTC_ASN1_INTEGER, 1UL, key->x,
LTC_ASN1_EOL, 0UL, NULL);
}
else {
flags[0] = 0;
err = der_encode_sequence_multi(out, outlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &version,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->prime,
LTC_ASN1_INTEGER, 1UL, key->base,
LTC_ASN1_INTEGER, 1UL, key->y,
LTC_ASN1_EOL, 0UL, NULL);
}
return err;
}
#endif 
