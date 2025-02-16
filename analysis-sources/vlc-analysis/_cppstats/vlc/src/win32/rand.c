#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_rand.h>
#if VLC_WINSTORE_APP
#include <bcrypt.h>
#else
#include <wincrypt.h>
#endif
void vlc_rand_bytes (void *buf, size_t len)
{
#if VLC_WINSTORE_APP
BCRYPT_ALG_HANDLE algo_handle;
NTSTATUS ret = BCryptOpenAlgorithmProvider(&algo_handle, BCRYPT_RNG_ALGORITHM,
MS_PRIMITIVE_PROVIDER, 0);
if (BCRYPT_SUCCESS(ret))
{
BCryptGenRandom(algo_handle, buf, len, 0);
BCryptCloseAlgorithmProvider(algo_handle, 0);
}
#else
size_t count = len;
uint8_t *p_buf = (uint8_t *)buf;
while (count > 0)
{
unsigned int val;
val = rand();
if (count < sizeof (val))
{
memcpy (p_buf, &val, count);
break;
}
memcpy (p_buf, &val, sizeof (val));
count -= sizeof (val);
p_buf += sizeof (val);
}
HCRYPTPROV hProv;
if( CryptAcquireContext(
&hProv, 
NULL, 
MS_DEF_PROV, 
PROV_RSA_FULL, 
CRYPT_VERIFYCONTEXT) ) 
{
CryptGenRandom(hProv, len, buf);
CryptReleaseContext(hProv, 0);
}
#endif 
}
