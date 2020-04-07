#include "tomcrypt_private.h"
#include <stdarg.h>
#if defined(LTC_F9_MODE)
int f9_memory_multi(int cipher,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...)
{
int err;
f9_state *f9;
va_list args;
const unsigned char *curptr;
unsigned long curlen;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
f9 = XMALLOC(sizeof(f9_state));
if (f9 == NULL) {
return CRYPT_MEM;
}
if ((err = f9_init(f9, cipher, key, keylen)) != CRYPT_OK) {
goto LBL_ERR;
}
va_start(args, inlen);
curptr = in;
curlen = inlen;
for (;;) {
if ((err = f9_process(f9, curptr, curlen)) != CRYPT_OK) {
goto LBL_ERR;
}
curptr = va_arg(args, const unsigned char*);
if (curptr == NULL) {
break;
}
curlen = va_arg(args, unsigned long);
}
if ((err = f9_done(f9, out, outlen)) != CRYPT_OK) {
goto LBL_ERR;
}
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(f9, sizeof(f9_state));
#endif
XFREE(f9);
va_end(args);
return err;
}
#endif
