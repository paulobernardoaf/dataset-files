







#include "tomcrypt_private.h"







#if defined(LTC_DER)









int der_encode_integer(void *num, unsigned char *out, unsigned long *outlen)
{
unsigned long tmplen, y, len;
int err, leading_zero;

LTC_ARGCHK(num != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if ((err = der_length_integer(num, &tmplen)) != CRYPT_OK) {
return err;
}

if (*outlen < tmplen) {
*outlen = tmplen;
return CRYPT_BUFFER_OVERFLOW;
}

if (mp_cmp_d(num, 0) != LTC_MP_LT) {

if ((mp_count_bits(num) & 7) == 0 || mp_iszero(num) == LTC_MP_YES) {
leading_zero = 1;
} else {
leading_zero = 0;
}


y = mp_unsigned_bin_size(num) + leading_zero;
} else {
leading_zero = 0;
y = mp_count_bits(num);
y = y + (8 - (y & 7));
y = y >> 3;
if (((mp_cnt_lsb(num)+1)==mp_count_bits(num)) && ((mp_count_bits(num)&7)==0)) --y;
}


*out++ = 0x02;
len = *outlen - 1;
if ((err = der_encode_asn1_length(y, out, &len)) != CRYPT_OK) {
return err;
}
out += len;


if (leading_zero) {
*out++ = 0x00;
}


if (mp_cmp_d(num, 0) == LTC_MP_GT) {

if ((err = mp_to_unsigned_bin(num, out)) != CRYPT_OK) {
return err;
}
} else if (mp_iszero(num) != LTC_MP_YES) {
void *tmp;


if (mp_init(&tmp) != CRYPT_OK) {
return CRYPT_MEM;
}


y = mp_count_bits(num);
y = y + (8 - (y & 7));
if (((mp_cnt_lsb(num)+1)==mp_count_bits(num)) && ((mp_count_bits(num)&7)==0)) y -= 8;
if (mp_2expt(tmp, y) != CRYPT_OK || mp_add(tmp, num, tmp) != CRYPT_OK) {
mp_clear(tmp);
return CRYPT_MEM;
}
if ((err = mp_to_unsigned_bin(tmp, out)) != CRYPT_OK) {
mp_clear(tmp);
return err;
}
mp_clear(tmp);
}


*outlen = tmplen;
return CRYPT_OK;
}

#endif




