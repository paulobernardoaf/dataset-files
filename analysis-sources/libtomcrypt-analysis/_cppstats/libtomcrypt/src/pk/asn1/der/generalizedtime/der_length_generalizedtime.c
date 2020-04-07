#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_length_generalizedtime(const ltc_generalizedtime *gtime, unsigned long *outlen)
{
LTC_ARGCHK(outlen != NULL);
LTC_ARGCHK(gtime != NULL);
if (gtime->fs == 0) {
*outlen = 2 + 14 + 1;
} else {
unsigned long len = 2 + 14 + 1;
unsigned fs = gtime->fs;
do {
fs /= 10;
len++;
} while(fs != 0);
if (gtime->off_hh == 0 && gtime->off_mm == 0) {
len += 1;
}
else {
len += 5;
}
*outlen = len;
}
return CRYPT_OK;
}
#endif
