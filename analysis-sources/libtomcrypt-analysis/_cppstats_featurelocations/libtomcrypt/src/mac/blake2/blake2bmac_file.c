








#include "tomcrypt_private.h"

#if defined(LTC_BLAKE2BMAC)










int blake2bmac_file(const char *fname, const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen)
{
#if defined(LTC_NO_FILE)
LTC_UNUSED_PARAM(fname);
LTC_UNUSED_PARAM(key);
LTC_UNUSED_PARAM(keylen);
LTC_UNUSED_PARAM(mac);
LTC_UNUSED_PARAM(maclen);
return CRYPT_NOP;
#else
blake2bmac_state st;
FILE *in;
unsigned char *buf;
size_t x;
int err;

LTC_ARGCHK(fname != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(mac != NULL);
LTC_ARGCHK(maclen != NULL);

if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
return CRYPT_MEM;
}

if ((err = blake2bmac_init(&st, *maclen, key, keylen)) != CRYPT_OK) {
goto LBL_ERR;
}

in = fopen(fname, "rb");
if (in == NULL) {
err = CRYPT_FILE_NOTFOUND;
goto LBL_ERR;
}

do {
x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
if ((err = blake2bmac_process(&st, buf, (unsigned long)x)) != CRYPT_OK) {
fclose(in);
goto LBL_CLEANBUF;
}
} while (x == LTC_FILE_READ_BUFSIZE);

if (fclose(in) != 0) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}

err = blake2bmac_done(&st, mac, maclen);

LBL_CLEANBUF:
zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&st, sizeof(blake2bmac_state));
#endif
XFREE(buf);
return err;
#endif
}

#endif




