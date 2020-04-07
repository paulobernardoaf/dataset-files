#include "tomcrypt_private.h"
#if defined(LTC_MDH)
int dh_set_pg_dhparam(const unsigned char *dhparam, unsigned long dhparamlen, dh_key *key)
{
int err;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
LTC_ARGCHK(dhparam != NULL);
LTC_ARGCHK(dhparamlen > 0);
if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
return err;
}
if ((err = der_decode_sequence_multi(dhparam, dhparamlen,
LTC_ASN1_INTEGER, 1UL, key->prime,
LTC_ASN1_INTEGER, 1UL, key->base,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto LBL_ERR;
}
return CRYPT_OK;
LBL_ERR:
dh_free(key);
return err;
}
#endif 
