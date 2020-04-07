#include "tomcrypt_private.h"
#if defined(LTC_OCB3_MODE)
int ocb3_int_ntz(unsigned long x)
{
int c;
x &= 0xFFFFFFFFUL;
c = 0;
while ((x & 1) == 0) {
++c;
x >>= 1;
}
return c;
}
#endif
