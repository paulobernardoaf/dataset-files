#include "tomcrypt.h"
#if defined(LTC_GCM_MODE)
#if !defined(LTC_NO_FILE)
int gcm_file( int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *IV, unsigned long IVlen,
const unsigned char *adata, unsigned long adatalen,
const char *in,
const char *out,
unsigned long taglen,
int direction,
int *res)
{
int err;
FILE *f_in = NULL, *f_out = NULL;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(res != NULL);
*res = 0;
f_in = fopen(in, "rb");
if (f_in == NULL) {
err = CRYPT_FILE_NOTFOUND;
goto LBL_ERR;
}
f_out = fopen(out, "w+b");
if (f_out == NULL) {
err = CRYPT_FILE_NOTFOUND;
goto LBL_ERR;
}
err = gcm_filehandle(cipher, key, keylen, IV, IVlen, adata, adatalen, f_in, f_out, taglen, direction, res);
LBL_ERR:
if (f_out != NULL && fclose(f_out) != 0) {
err = CRYPT_ERROR;
}
if (*res != 1) {
remove(out);
}
if (f_in != NULL && fclose(f_in) != 0) {
err = CRYPT_ERROR;
}
return err;
}
#endif
#endif
