#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_get_size(const ecc_key *key)
{
if (key == NULL) {
return INT_MAX;
}
return key->dp.size;
}
#endif
