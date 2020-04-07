












#include "tomcrypt_private.h"

#if defined(LTC_OCB3_MODE)








void ocb3_int_xor_blocks(unsigned char *out, const unsigned char *block_a, const unsigned char *block_b, unsigned long block_len)
{
int x;
if (out == block_a) {
for (x = 0; x < (int)block_len; x++) out[x] ^= block_b[x];
}
else {
for (x = 0; x < (int)block_len; x++) out[x] = block_a[x] ^ block_b[x];
}
}

#endif




