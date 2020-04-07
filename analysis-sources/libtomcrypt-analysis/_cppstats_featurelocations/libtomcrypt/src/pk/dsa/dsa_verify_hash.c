







#include "tomcrypt_private.h"







#if defined(LTC_MDSA)











int dsa_verify_hash_raw( void *r, void *s,
const unsigned char *hash, unsigned long hashlen,
int *stat, const dsa_key *key)
{
void *w, *v, *u1, *u2;
int err;

LTC_ARGCHK(r != NULL);
LTC_ARGCHK(s != NULL);
LTC_ARGCHK(stat != NULL);
LTC_ARGCHK(key != NULL);


*stat = 0;


if ((err = mp_init_multi(&w, &v, &u1, &u2, NULL)) != CRYPT_OK) {
return err;
}


if (mp_cmp_d(r, 0) != LTC_MP_GT || mp_cmp_d(s, 0) != LTC_MP_GT || mp_cmp(r, key->q) != LTC_MP_LT || mp_cmp(s, key->q) != LTC_MP_LT) {
err = CRYPT_INVALID_PACKET;
goto error;
}


hashlen = MIN(hashlen, (unsigned long)(key->qord));


if ((err = mp_invmod(s, key->q, w)) != CRYPT_OK) { goto error; }


if ((err = mp_read_unsigned_bin(u1, (unsigned char *)hash, hashlen)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(u1, w, key->q, u1)) != CRYPT_OK) { goto error; }


if ((err = mp_mulmod(r, w, key->q, u2)) != CRYPT_OK) { goto error; }


if ((err = mp_exptmod(key->g, u1, key->p, u1)) != CRYPT_OK) { goto error; }
if ((err = mp_exptmod(key->y, u2, key->p, u2)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(u1, u2, key->p, v)) != CRYPT_OK) { goto error; }
if ((err = mp_mod(v, key->q, v)) != CRYPT_OK) { goto error; }


if (mp_cmp(r, v) == LTC_MP_EQ) {
*stat = 1;
}

err = CRYPT_OK;
error:
mp_clear_multi(w, v, u1, u2, NULL);
return err;
}











int dsa_verify_hash(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
int *stat, const dsa_key *key)
{
int err;
void *r, *s;
ltc_asn1_list sig_seq[2];
unsigned long reallen = 0;

LTC_ARGCHK(stat != NULL);
*stat = 0; 

if ((err = mp_init_multi(&r, &s, NULL)) != CRYPT_OK) {
return err;
}

LTC_SET_ASN1(sig_seq, 0, LTC_ASN1_INTEGER, r, 1UL);
LTC_SET_ASN1(sig_seq, 1, LTC_ASN1_INTEGER, s, 1UL);

err = der_decode_sequence_strict(sig, siglen, sig_seq, 2);
if (err != CRYPT_OK) {
goto LBL_ERR;
}

err = der_length_sequence(sig_seq, 2, &reallen);
if (err != CRYPT_OK || reallen != siglen) {
goto LBL_ERR;
}


err = dsa_verify_hash_raw(r, s, hash, hashlen, stat, key);

LBL_ERR:
mp_clear_multi(r, s, NULL);
return err;
}

#endif





