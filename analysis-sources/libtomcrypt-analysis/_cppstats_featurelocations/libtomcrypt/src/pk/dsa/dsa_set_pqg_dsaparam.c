







#include "tomcrypt_private.h"


#if defined(LTC_MDSA)











int dsa_set_pqg_dsaparam(const unsigned char *dsaparam, unsigned long dsaparamlen,
dsa_key *key)
{
int err, stat;

LTC_ARGCHK(dsaparam != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);


err = mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL);
if (err != CRYPT_OK) return err;

if ((err = der_decode_sequence_multi(dsaparam, dsaparamlen,
LTC_ASN1_INTEGER, 1UL, key->p,
LTC_ASN1_INTEGER, 1UL, key->q,
LTC_ASN1_INTEGER, 1UL, key->g,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto LBL_ERR;
}

key->qord = mp_unsigned_bin_size(key->q);


if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) {
goto LBL_ERR;
}
if (stat == 0) {
err = CRYPT_INVALID_PACKET;
goto LBL_ERR;
}

return CRYPT_OK;

LBL_ERR:
dsa_free(key);
return err;
}

#endif




