







#include "tomcrypt_private.h"






#if defined(LTC_LRW_MODE)








int lrw_setiv(const unsigned char *IV, unsigned long len, symmetric_LRW *lrw)
{
int err;
#if defined(LTC_LRW_TABLES)
unsigned char T[16];
int x, y;
#endif
LTC_ARGCHK(IV != NULL);
LTC_ARGCHK(lrw != NULL);

if (len != 16) {
return CRYPT_INVALID_ARG;
}

if ((err = cipher_is_valid(lrw->cipher)) != CRYPT_OK) {
return err;
}


XMEMCPY(lrw->IV, IV, 16);


if (cipher_descriptor[lrw->cipher].accel_lrw_encrypt != NULL && cipher_descriptor[lrw->cipher].accel_lrw_decrypt != NULL) {

return CRYPT_OK;
}

#if defined(LTC_LRW_TABLES)
XMEMCPY(T, &lrw->PC[0][IV[0]][0], 16);
for (x = 1; x < 16; x++) {
#if defined(LTC_FAST)
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(T + y)) ^= *(LTC_FAST_TYPE_PTR_CAST(&lrw->PC[x][IV[x]][y]));
}
#else
for (y = 0; y < 16; y++) {
T[y] ^= lrw->PC[x][IV[x]][y];
}
#endif
}
XMEMCPY(lrw->pad, T, 16);
#else
gcm_gf_mult(lrw->tweak, IV, lrw->pad);
#endif

return CRYPT_OK;
}


#endif



