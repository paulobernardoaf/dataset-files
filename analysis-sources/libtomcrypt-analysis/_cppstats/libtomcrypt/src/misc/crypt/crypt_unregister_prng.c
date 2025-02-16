#include "tomcrypt_private.h"
int unregister_prng(const struct ltc_prng_descriptor *prng)
{
int x;
LTC_ARGCHK(prng != NULL);
LTC_MUTEX_LOCK(&ltc_prng_mutex);
for (x = 0; x < TAB_SIZE; x++) {
if (XMEMCMP(&prng_descriptor[x], prng, sizeof(struct ltc_prng_descriptor)) == 0) {
prng_descriptor[x].name = NULL;
LTC_MUTEX_UNLOCK(&ltc_prng_mutex);
return CRYPT_OK;
}
}
LTC_MUTEX_UNLOCK(&ltc_prng_mutex);
return CRYPT_ERROR;
}
