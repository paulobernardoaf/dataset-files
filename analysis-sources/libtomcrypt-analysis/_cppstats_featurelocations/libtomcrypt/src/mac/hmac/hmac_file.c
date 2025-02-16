







#include "tomcrypt_private.h"






#if defined(LTC_HMAC)











int hmac_file(int hash, const char *fname,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen)
{
#if defined(LTC_NO_FILE)
LTC_UNUSED_PARAM(hash);
LTC_UNUSED_PARAM(fname);
LTC_UNUSED_PARAM(key);
LTC_UNUSED_PARAM(keylen);
LTC_UNUSED_PARAM(out);
LTC_UNUSED_PARAM(outlen);
return CRYPT_NOP;
#else
hmac_state hmac;
FILE *in;
unsigned char *buf;
size_t x;
int err;

LTC_ARGCHK(fname != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);

if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
return CRYPT_MEM;
}

if ((err = hash_is_valid(hash)) != CRYPT_OK) {
goto LBL_ERR;
}

if ((err = hmac_init(&hmac, hash, key, keylen)) != CRYPT_OK) {
goto LBL_ERR;
}

in = fopen(fname, "rb");
if (in == NULL) {
err = CRYPT_FILE_NOTFOUND;
goto LBL_ERR;
}

do {
x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
if ((err = hmac_process(&hmac, buf, (unsigned long)x)) != CRYPT_OK) {
fclose(in); 
goto LBL_CLEANBUF;
}
} while (x == LTC_FILE_READ_BUFSIZE);

if (fclose(in) != 0) {
err = CRYPT_ERROR;
goto LBL_CLEANBUF;
}

err = hmac_done(&hmac, out, outlen);

LBL_CLEANBUF:
zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(&hmac, sizeof(hmac_state));
#endif
XFREE(buf);
return err;
#endif
}

#endif




