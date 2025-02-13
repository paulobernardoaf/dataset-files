#include "tomcrypt_private.h"
#if defined(LTC_MRSA)
int rsa_sign_saltlen_get_max_ex(int padding, int hash_idx, const rsa_key *key)
{
int ret = INT_MAX;
LTC_ARGCHK(key != NULL);
if ((hash_is_valid(hash_idx) == CRYPT_OK) &&
(padding == LTC_PKCS_1_PSS))
{
ret = rsa_get_size(key);
if (ret < INT_MAX)
{
ret -= (hash_descriptor[hash_idx].hashsize + 2);
} 
} 
return ret;
}
#endif
