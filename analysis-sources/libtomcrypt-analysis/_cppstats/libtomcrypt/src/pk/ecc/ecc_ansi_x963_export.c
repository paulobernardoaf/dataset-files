#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_ansi_x963_export(const ecc_key *key, unsigned char *out, unsigned long *outlen)
{
return ecc_get_key(out, outlen, PK_PUBLIC, key);
}
#endif
