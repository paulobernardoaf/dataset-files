







#include "tomcrypt_private.h"






#if defined(LTC_MDSA)





void dsa_free(dsa_key *key)
{
LTC_ARGCHKVD(key != NULL);
mp_cleanup_multi(&key->y, &key->x, &key->q, &key->g, &key->p, NULL);
key->type = key->qord = 0;
}

#endif




