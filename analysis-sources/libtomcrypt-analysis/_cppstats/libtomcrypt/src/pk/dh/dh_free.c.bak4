








#include "tomcrypt_private.h"

#if defined(LTC_MDH)





void dh_free(dh_key *key)
{
LTC_ARGCHKVD(key != NULL);
mp_cleanup_multi(&key->prime, &key->base, &key->y, &key->x, NULL);
}

#endif 




