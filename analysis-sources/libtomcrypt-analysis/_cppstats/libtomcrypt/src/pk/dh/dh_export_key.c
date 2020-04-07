#include "tomcrypt_private.h"
#if defined(LTC_MDH)
int dh_export_key(void *out, unsigned long *outlen, int type, const dh_key *key)
{
unsigned long len;
void *k;
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
LTC_ARGCHK(key != NULL);
k = (type == PK_PRIVATE) ? key->x : key->y;
len = mp_unsigned_bin_size(k);
if (*outlen < len) {
*outlen = len;
return CRYPT_BUFFER_OVERFLOW;
}
*outlen = len;
return mp_to_unsigned_bin(k, out);
}
#endif 
