







#include "tomcrypt_private.h"











int unregister_cipher(const struct ltc_cipher_descriptor *cipher)
{
int x;

LTC_ARGCHK(cipher != NULL);


LTC_MUTEX_LOCK(&ltc_cipher_mutex);
for (x = 0; x < TAB_SIZE; x++) {
if (XMEMCMP(&cipher_descriptor[x], cipher, sizeof(struct ltc_cipher_descriptor)) == 0) {
cipher_descriptor[x].name = NULL;
cipher_descriptor[x].ID = 255;
LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
return CRYPT_OK;
}
}
LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
return CRYPT_ERROR;
}




