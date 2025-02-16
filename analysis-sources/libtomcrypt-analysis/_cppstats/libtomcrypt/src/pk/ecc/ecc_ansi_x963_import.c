#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_ansi_x963_import(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
return ecc_ansi_x963_import_ex(in, inlen, key, NULL);
}
int ecc_ansi_x963_import_ex(const unsigned char *in, unsigned long inlen, ecc_key *key, const ltc_ecc_curve *cu)
{
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
if ((inlen & 1) == 0) {
return CRYPT_INVALID_ARG;
}
if (cu == NULL) {
if ((err = ecc_set_curve_by_size((inlen-1)>>1, key)) != CRYPT_OK) { return err; }
}
else {
if ((err = ecc_set_curve(cu, key)) != CRYPT_OK) { return err; }
}
if ((err = ecc_set_key((unsigned char *)in, inlen, PK_PUBLIC, key)) != CRYPT_OK) { return err; }
return CRYPT_OK;
}
#endif
