#include "tomcrypt_private.h"
#if defined(LTC_MDH)
int dh_set_pg(const unsigned char *p, unsigned long plen,
const unsigned char *g, unsigned long glen,
dh_key *key)
{
int err;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(p != NULL);
LTC_ARGCHK(g != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
return err;
}
if ((err = mp_read_unsigned_bin(key->base, (unsigned char*)g, glen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->prime, (unsigned char*)p, plen)) != CRYPT_OK) { goto LBL_ERR; }
return CRYPT_OK;
LBL_ERR:
dh_free(key);
return err;
}
int dh_set_pg_groupsize(int groupsize, dh_key *key)
{
int err, i;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
LTC_ARGCHK(groupsize > 0);
for (i = 0; (groupsize > ltc_dh_sets[i].size) && (ltc_dh_sets[i].size != 0); i++);
if (ltc_dh_sets[i].size == 0) return CRYPT_INVALID_KEYSIZE;
if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
return err;
}
if ((err = mp_read_radix(key->base, ltc_dh_sets[i].base, 16)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_radix(key->prime, ltc_dh_sets[i].prime, 16)) != CRYPT_OK) { goto LBL_ERR; }
return CRYPT_OK;
LBL_ERR:
dh_free(key);
return err;
}
int dh_set_key(const unsigned char *in, unsigned long inlen, int type, dh_key *key)
{
int err;
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
if (type == PK_PRIVATE) {
key->type = PK_PRIVATE;
if ((err = mp_read_unsigned_bin(key->x, (unsigned char*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK) { goto LBL_ERR; }
}
else {
key->type = PK_PUBLIC;
if ((err = mp_read_unsigned_bin(key->y, (unsigned char*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
}
if ((err = dh_check_pubkey(key)) != CRYPT_OK) {
goto LBL_ERR;
}
return CRYPT_OK;
LBL_ERR:
dh_free(key);
return err;
}
#endif 
