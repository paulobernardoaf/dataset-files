







#include "tomcrypt_private.h"











int find_hash_id(unsigned char ID)
{
int x;
LTC_MUTEX_LOCK(&ltc_hash_mutex);
for (x = 0; x < TAB_SIZE; x++) {
if (hash_descriptor[x].ID == ID) {
x = (hash_descriptor[x].name == NULL) ? -1 : x;
LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
return x;
}
}
LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
return -1;
}




