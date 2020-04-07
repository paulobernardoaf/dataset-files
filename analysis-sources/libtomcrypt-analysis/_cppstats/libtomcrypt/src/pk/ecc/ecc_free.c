#include "tomcrypt_private.h"
#if defined(LTC_MECC)
void ecc_free(ecc_key *key)
{
LTC_ARGCHKVD(key != NULL);
mp_cleanup_multi(&key->dp.prime, &key->dp.order,
&key->dp.A, &key->dp.B,
&key->dp.base.x, &key->dp.base.y, &key->dp.base.z,
&key->pubkey.x, &key->pubkey.y, &key->pubkey.z,
&key->k, NULL);
}
#endif
