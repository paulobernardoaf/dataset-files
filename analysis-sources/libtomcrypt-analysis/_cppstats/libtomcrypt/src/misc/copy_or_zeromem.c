#include "tomcrypt_private.h"
void copy_or_zeromem(const unsigned char* src, unsigned char* dest, unsigned long len, int coz)
{
unsigned long y;
#if defined(LTC_FAST)
unsigned long z;
LTC_FAST_TYPE fastMask = ~(LTC_FAST_TYPE)0; 
#endif
unsigned char mask = 0xff; 
LTC_ARGCHKVD(src != NULL);
LTC_ARGCHKVD(dest != NULL);
if (coz != 0) coz = 1;
y = 0;
mask *= 1 - coz; 
#if defined(LTC_FAST)
fastMask *= 1 - coz;
if (len & ~15) {
for (; y < (len & ~15); y += 16) {
for (z = 0; z < 16; z += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(&dest[y+z])) = *(LTC_FAST_TYPE_PTR_CAST(&src[y+z])) & fastMask;
}
}
}
#endif
for (; y < len; y++) {
dest[y] = src[y] & mask;
}
#if defined(LTC_CLEAN_STACK)
#if defined(LTC_FAST)
fastMask = 0;
#endif
mask = 0;
#endif
}
