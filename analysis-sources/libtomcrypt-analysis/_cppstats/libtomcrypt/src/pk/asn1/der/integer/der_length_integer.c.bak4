







#include "tomcrypt_private.h"







#if defined(LTC_DER)






int der_length_integer(void *num, unsigned long *outlen)
{
unsigned long z, len;
int leading_zero, err;

LTC_ARGCHK(num != NULL);
LTC_ARGCHK(outlen != NULL);

if (mp_cmp_d(num, 0) != LTC_MP_LT) {



if ((mp_count_bits(num) & 7) == 0 || mp_iszero(num) == LTC_MP_YES) {
leading_zero = 1;
} else {
leading_zero = 0;
}


len = leading_zero + mp_unsigned_bin_size(num);
} else {


z = mp_count_bits(num);
z = z + (8 - (z & 7));
if (((mp_cnt_lsb(num)+1)==mp_count_bits(num)) && ((mp_count_bits(num)&7)==0)) --z;
len = z >> 3;
}

if ((err = der_length_asn1_length(len, &z)) != CRYPT_OK) {
return err;
}
*outlen = 1 + z + len;

return CRYPT_OK;
}

#endif




