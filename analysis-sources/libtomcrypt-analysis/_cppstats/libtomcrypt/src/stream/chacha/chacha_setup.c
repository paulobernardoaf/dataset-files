#include "tomcrypt_private.h"
#if defined(LTC_CHACHA)
static const char * const sigma = "expand 32-byte k";
static const char * const tau = "expand 16-byte k";
int chacha_setup(chacha_state *st, const unsigned char *key, unsigned long keylen, int rounds)
{
const char *constants;
LTC_ARGCHK(st != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(keylen == 32 || keylen == 16);
if (rounds == 0) rounds = 20;
LOAD32L(st->input[4], key + 0);
LOAD32L(st->input[5], key + 4);
LOAD32L(st->input[6], key + 8);
LOAD32L(st->input[7], key + 12);
if (keylen == 32) { 
key += 16;
constants = sigma;
} else { 
constants = tau;
}
LOAD32L(st->input[8], key + 0);
LOAD32L(st->input[9], key + 4);
LOAD32L(st->input[10], key + 8);
LOAD32L(st->input[11], key + 12);
LOAD32L(st->input[0], constants + 0);
LOAD32L(st->input[1], constants + 4);
LOAD32L(st->input[2], constants + 8);
LOAD32L(st->input[3], constants + 12);
st->rounds = rounds; 
st->ivlen = 0; 
return CRYPT_OK;
}
#endif
