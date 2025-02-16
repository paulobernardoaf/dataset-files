







#include "tomcrypt_private.h"






#if defined(LTC_OMAC)











int omac_file(int cipher,
const unsigned char *key, unsigned long keylen,
const char *filename,
unsigned char *out, unsigned long *outlen)
{
#if defined(LTC_NO_FILE)
LTC_UNUSED_PARAM(cipher);
LTC_UNUSED_PARAM(key);
LTC_UNUSED_PARAM(keylen);
LTC_UNUSED_PARAM(filename);
LTC_UNUSED_PARAM(out);
LTC_UNUSED_PARAM(outlen);
return CRYPT_NOP;
#else
size_t x;
int err;
omac_state omac;
FILE *in;
unsigned char *buf;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(filename != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);

if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
return CRYPT_MEM;
}

if ((err = omac_init(&omac, cipher, key, keylen)) != CRYPT_OK) {
goto LBL_ERR;
}

in = fopen(filename, "rb");
if (in == NULL) {
err = CRYPT_FILE_NOTFOUND;
goto LBL_ERR;
}

do {
x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
if ((err = omac_process(&omac, buf, (unsigned long)x)) != CRYPT_OK) {
fclose(in);
goto LBL_CLEANBUF;
}
} while (x == LTC_FILE_READ_BUFSIZE);

if (fclose(in) != 0) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}

err = omac_done(&omac, out, outlen);

LBL_CLEANBUF:
zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&omac, sizeof(omac_state));
#endif
XFREE(buf);
return err;
#endif
}

#endif




