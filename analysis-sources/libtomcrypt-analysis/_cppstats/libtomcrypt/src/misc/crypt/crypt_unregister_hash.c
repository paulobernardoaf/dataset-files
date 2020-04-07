#include "tomcrypt_private.h"
int unregister_hash(const struct ltc_hash_descriptor *hash)
{
int x;
LTC_ARGCHK(hash != NULL);
LTC_MUTEX_LOCK(&ltc_hash_mutex);
for (x = 0; x < TAB_SIZE; x++) {
if (XMEMCMP(&hash_descriptor[x], hash, sizeof(struct ltc_hash_descriptor)) == 0) {
hash_descriptor[x].name = NULL;
LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
return CRYPT_OK;
}
}
LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
return CRYPT_ERROR;
}
