







#include "tomcrypt_private.h"






#if defined(LTC_DER)






int der_length_bit_string(unsigned long nbits, unsigned long *outlen)
{
unsigned long nbytes, x;
int err;

LTC_ARGCHK(outlen != NULL);


nbytes = (nbits >> 3) + ((nbits & 7) ? 1 : 0) + 1;

if ((err = der_length_asn1_length(nbytes, &x)) != CRYPT_OK) {
return err;
}
*outlen = 1 + x + nbytes;

return CRYPT_OK;
}

#endif





