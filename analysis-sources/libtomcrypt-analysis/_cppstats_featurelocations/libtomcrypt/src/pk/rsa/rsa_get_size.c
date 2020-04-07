







#include "tomcrypt_private.h"






#if defined(LTC_MRSA)






int rsa_get_size(const rsa_key *key)
{
int ret = INT_MAX;
LTC_ARGCHK(key != NULL);

if (key)
{
ret = mp_unsigned_bin_size(key->N);
} 

return ret;
}

#endif




