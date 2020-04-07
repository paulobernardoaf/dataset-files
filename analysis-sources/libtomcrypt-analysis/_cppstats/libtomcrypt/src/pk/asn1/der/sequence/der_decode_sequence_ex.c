#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_decode_sequence_ex(const unsigned char *in, unsigned long inlen,
ltc_asn1_list *list, unsigned long outlen, unsigned int flags)
{
return der_decode_custom_type_ex(in, inlen, NULL, list, outlen, flags);
}
#endif
