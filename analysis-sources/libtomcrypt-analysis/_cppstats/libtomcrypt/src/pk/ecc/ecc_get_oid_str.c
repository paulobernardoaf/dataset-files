#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_get_oid_str(char *out, unsigned long *outlen, const ecc_key *key)
{
LTC_ARGCHK(key != NULL);
return pk_oid_num_to_str(key->dp.oid, key->dp.oidlen, out, outlen);
}
#endif
