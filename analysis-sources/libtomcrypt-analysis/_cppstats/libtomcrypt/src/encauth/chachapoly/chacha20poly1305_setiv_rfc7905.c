#include "tomcrypt_private.h"
#if defined(LTC_CHACHA20POLY1305_MODE)
int chacha20poly1305_setiv_rfc7905(chacha20poly1305_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 sequence_number)
{
int i;
unsigned char combined_iv[12] = { 0 };
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(iv != NULL);
LTC_ARGCHK(ivlen == 12);
STORE64L(sequence_number, combined_iv + 4);
for (i = 0; i < 12; i++) combined_iv[i] = iv[i] ^ combined_iv[i];
return chacha20poly1305_setiv(st, combined_iv, 12);
}
#endif
