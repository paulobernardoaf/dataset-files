







#include "tomcrypt_private.h"


#if defined(LTC_MDSA)












int dsa_set_pqg(const unsigned char *p, unsigned long plen,
const unsigned char *q, unsigned long qlen,
const unsigned char *g, unsigned long glen,
dsa_key *key)
{
int err, stat;

LTC_ARGCHK(p != NULL);
LTC_ARGCHK(q != NULL);
LTC_ARGCHK(g != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);


err = mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL);
if (err != CRYPT_OK) return err;

if ((err = mp_read_unsigned_bin(key->p, (unsigned char *)p , plen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->g, (unsigned char *)g , glen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->q, (unsigned char *)q , qlen)) != CRYPT_OK) { goto LBL_ERR; }

key->qord = mp_unsigned_bin_size(key->q);


if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) { goto LBL_ERR; }
if (stat == 0) {
err = CRYPT_INVALID_PACKET;
goto LBL_ERR;
}

return CRYPT_OK;

LBL_ERR:
dsa_free(key);
return err;
}












int dsa_set_key(const unsigned char *in, unsigned long inlen, int type, dsa_key *key)
{
int err, stat = 0;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(key->x != NULL);
LTC_ARGCHK(key->y != NULL);
LTC_ARGCHK(key->p != NULL);
LTC_ARGCHK(key->g != NULL);
LTC_ARGCHK(key->q != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);

if (type == PK_PRIVATE) {
key->type = PK_PRIVATE;
if ((err = mp_read_unsigned_bin(key->x, (unsigned char *)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_exptmod(key->g, key->x, key->p, key->y)) != CRYPT_OK) { goto LBL_ERR; }
}
else {
key->type = PK_PUBLIC;
if ((err = mp_read_unsigned_bin(key->y, (unsigned char *)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
}

if ((err = dsa_int_validate_xy(key, &stat)) != CRYPT_OK) { goto LBL_ERR; }
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




