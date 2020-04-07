







#include "tomcrypt_private.h"






#if defined(LTC_DER)







int der_decode_asn1_length(const unsigned char *in, unsigned long *inlen, unsigned long *outlen)
{
unsigned long real_len, decoded_len, offset, i;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen != NULL);

if (*inlen < 1) {
return CRYPT_BUFFER_OVERFLOW;
}

real_len = in[0];

if (real_len < 128) {
decoded_len = real_len;
offset = 1;
} else {
real_len &= 0x7F;
if (real_len == 0) {
return CRYPT_PK_ASN1_ERROR;
}
if (real_len > sizeof(decoded_len)) {
return CRYPT_OVERFLOW;
}
if (real_len > (*inlen - 1)) {
return CRYPT_BUFFER_OVERFLOW;
}
decoded_len = 0;
offset = 1 + real_len;
for (i = 0; i < real_len; i++) {
decoded_len = (decoded_len << 8) | in[1 + i];
}
}

if (outlen != NULL) *outlen = decoded_len;
if (decoded_len > (*inlen - offset)) return CRYPT_OVERFLOW;
*inlen = offset;

return CRYPT_OK;
}

#endif




