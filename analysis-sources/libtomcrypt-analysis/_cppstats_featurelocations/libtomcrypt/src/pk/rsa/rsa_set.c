







#include "tomcrypt_private.h"


#if defined(LTC_MRSA)













int rsa_set_key(const unsigned char *N, unsigned long Nlen,
const unsigned char *e, unsigned long elen,
const unsigned char *d, unsigned long dlen,
rsa_key *key)
{
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(N != NULL);
LTC_ARGCHK(e != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);

if ((err = rsa_init(key)) != CRYPT_OK) return err;

if ((err = mp_read_unsigned_bin(key->N , (unsigned char *)N , Nlen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->e , (unsigned char *)e , elen)) != CRYPT_OK) { goto LBL_ERR; }
if (d && dlen) {
if ((err = mp_read_unsigned_bin(key->d , (unsigned char *)d , dlen)) != CRYPT_OK) { goto LBL_ERR; }
key->type = PK_PRIVATE;
}
else {
key->type = PK_PUBLIC;
}
return CRYPT_OK;

LBL_ERR:
rsa_free(key);
return err;
}













int rsa_set_factors(const unsigned char *p, unsigned long plen,
const unsigned char *q, unsigned long qlen,
rsa_key *key)
{
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(p != NULL);
LTC_ARGCHK(q != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);

if (key->type != PK_PRIVATE) return CRYPT_PK_TYPE_MISMATCH;

if ((err = mp_read_unsigned_bin(key->p , (unsigned char *)p , plen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->q , (unsigned char *)q , qlen)) != CRYPT_OK) { goto LBL_ERR; }
return CRYPT_OK;

LBL_ERR:
rsa_free(key);
return err;
}















int rsa_set_crt_params(const unsigned char *dP, unsigned long dPlen,
const unsigned char *dQ, unsigned long dQlen,
const unsigned char *qP, unsigned long qPlen,
rsa_key *key)
{
int err;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(dP != NULL);
LTC_ARGCHK(dQ != NULL);
LTC_ARGCHK(qP != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);

if (key->type != PK_PRIVATE) return CRYPT_PK_TYPE_MISMATCH;

if ((err = mp_read_unsigned_bin(key->dP, (unsigned char *)dP, dPlen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->dQ, (unsigned char *)dQ, dQlen)) != CRYPT_OK) { goto LBL_ERR; }
if ((err = mp_read_unsigned_bin(key->qP, (unsigned char *)qP, qPlen)) != CRYPT_OK) { goto LBL_ERR; }
return CRYPT_OK;

LBL_ERR:
rsa_free(key);
return err;
}

#endif 




