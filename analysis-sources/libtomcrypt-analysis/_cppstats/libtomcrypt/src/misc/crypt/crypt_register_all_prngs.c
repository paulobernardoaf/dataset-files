#include "tomcrypt_private.h"
#define REGISTER_PRNG(h) do {LTC_ARGCHK(register_prng(h) != -1); } while(0)
int register_all_prngs(void)
{
#if defined(LTC_YARROW)
REGISTER_PRNG(&yarrow_desc);
#endif
#if defined(LTC_FORTUNA)
REGISTER_PRNG(&fortuna_desc);
#endif
#if defined(LTC_RC4)
REGISTER_PRNG(&rc4_desc);
#endif
#if defined(LTC_CHACHA20_PRNG)
REGISTER_PRNG(&chacha20_prng_desc);
#endif
#if defined(LTC_SOBER128)
REGISTER_PRNG(&sober128_desc);
#endif
#if defined(LTC_SPRNG)
REGISTER_PRNG(&sprng_desc);
#endif
return CRYPT_OK;
}
