







#include "tomcrypt_private.h"











int find_cipher_id(unsigned char ID)
{
int x;
LTC_MUTEX_LOCK(&ltc_cipher_mutex);
for (x = 0; x < TAB_SIZE; x++) {
if (cipher_descriptor[x].ID == ID) {
x = (cipher_descriptor[x].name == NULL) ? -1 : x;
LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
return x;
}
}
LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
return -1;
}




