













#include "tomcrypt_private.h"

#if defined(LTC_CHACHA)









int chacha_ivctr64(chacha_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 counter)
{
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(iv != NULL);

LTC_ARGCHK(ivlen == 8);

st->input[12] = (ulong32)(counter & 0xFFFFFFFF);
st->input[13] = (ulong32)(counter >> 32);
LOAD32L(st->input[14], iv + 0);
LOAD32L(st->input[15], iv + 4);
st->ksleft = 0;
st->ivlen = ivlen;
return CRYPT_OK;
}

#endif




