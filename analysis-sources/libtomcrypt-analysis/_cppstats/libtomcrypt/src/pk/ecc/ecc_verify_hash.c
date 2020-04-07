#include "tomcrypt_private.h"
#if defined(LTC_MECC)
int ecc_verify_hash_ex(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
ecc_signature_type sigformat, int *stat, const ecc_key *key)
{
ecc_point *mG = NULL, *mQ = NULL;
void *r, *s, *v, *w, *u1, *u2, *e, *p, *m, *a, *a_plus3;
void *mu = NULL, *ma = NULL;
void *mp = NULL;
int err;
unsigned long pbits, pbytes, i, shift_right;
unsigned char ch, buf[MAXBLOCKSIZE];
LTC_ARGCHK(sig != NULL);
LTC_ARGCHK(hash != NULL);
LTC_ARGCHK(stat != NULL);
LTC_ARGCHK(key != NULL);
*stat = 0;
if ((err = mp_init_multi(&r, &s, &v, &w, &u1, &u2, &e, &a_plus3, NULL)) != CRYPT_OK) {
return err;
}
p = key->dp.order;
m = key->dp.prime;
a = key->dp.A;
if ((err = mp_add_d(a, 3, a_plus3)) != CRYPT_OK) {
goto error;
}
mG = ltc_ecc_new_point();
mQ = ltc_ecc_new_point();
if (mQ == NULL || mG == NULL) {
err = CRYPT_MEM;
goto error;
}
if (sigformat == LTC_ECCSIG_ANSIX962) {
if ((err = der_decode_sequence_multi_ex(sig, siglen, LTC_DER_SEQ_SEQUENCE | LTC_DER_SEQ_STRICT,
LTC_ASN1_INTEGER, 1UL, r,
LTC_ASN1_INTEGER, 1UL, s,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) { goto error; }
}
else if (sigformat == LTC_ECCSIG_RFC7518) {
i = mp_unsigned_bin_size(key->dp.order);
if (siglen != (2 * i)) {
err = CRYPT_INVALID_PACKET;
goto error;
}
if ((err = mp_read_unsigned_bin(r, (unsigned char *)sig, i)) != CRYPT_OK) { goto error; }
if ((err = mp_read_unsigned_bin(s, (unsigned char *)sig+i, i)) != CRYPT_OK) { goto error; }
}
else if (sigformat == LTC_ECCSIG_ETH27) {
if (pk_oid_cmp_with_ulong("1.3.132.0.10", key->dp.oid, key->dp.oidlen) != CRYPT_OK) {
err = CRYPT_ERROR; goto error;
}
if (siglen != 65) { 
err = CRYPT_INVALID_PACKET;
goto error;
}
if ((err = mp_read_unsigned_bin(r, (unsigned char *)sig, 32)) != CRYPT_OK) { goto error; }
if ((err = mp_read_unsigned_bin(s, (unsigned char *)sig+32, 32)) != CRYPT_OK) { goto error; }
}
#if defined(LTC_SSH)
else if (sigformat == LTC_ECCSIG_RFC5656) {
char name[64], name2[64];
unsigned long namelen = sizeof(name);
unsigned long name2len = sizeof(name2);
if ((err = ssh_decode_sequence_multi(sig, &siglen,
LTC_SSHDATA_STRING, name, &namelen,
LTC_SSHDATA_MPINT, r,
LTC_SSHDATA_MPINT, s,
LTC_SSHDATA_EOL, NULL)) != CRYPT_OK) { goto error; }
if ((err = ecc_ssh_ecdsa_encode_name(name2, &name2len, key)) != CRYPT_OK) { goto error; }
if ((namelen != name2len) || (XSTRCMP(name, name2) != 0)) {
err = CRYPT_INVALID_ARG;
goto error;
}
}
#endif
else {
err = CRYPT_ERROR;
goto error;
}
if (mp_cmp_d(r, 0) != LTC_MP_GT || mp_cmp_d(s, 0) != LTC_MP_GT ||
mp_cmp(r, p) != LTC_MP_LT || mp_cmp(s, p) != LTC_MP_LT) {
err = CRYPT_INVALID_PACKET;
goto error;
}
pbits = mp_count_bits(p);
pbytes = (pbits+7) >> 3;
if (pbits > hashlen*8) {
if ((err = mp_read_unsigned_bin(e, (unsigned char *)hash, hashlen)) != CRYPT_OK) { goto error; }
}
else if (pbits % 8 == 0) {
if ((err = mp_read_unsigned_bin(e, (unsigned char *)hash, pbytes)) != CRYPT_OK) { goto error; }
}
else {
shift_right = 8 - pbits % 8;
for (i=0, ch=0; i<pbytes; i++) {
buf[i] = ch;
ch = (hash[i] << (8-shift_right));
buf[i] = buf[i] ^ (hash[i] >> shift_right);
}
if ((err = mp_read_unsigned_bin(e, (unsigned char *)buf, pbytes)) != CRYPT_OK) { goto error; }
}
if ((err = mp_invmod(s, p, w)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(e, w, p, u1)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(r, w, p, u2)) != CRYPT_OK) { goto error; }
if ((err = ltc_ecc_copy_point(&key->dp.base, mG)) != CRYPT_OK) { goto error; }
if ((err = ltc_ecc_copy_point(&key->pubkey, mQ)) != CRYPT_OK) { goto error; }
if ((err = mp_montgomery_setup(m, &mp)) != CRYPT_OK) { goto error; }
if (mp_cmp(a_plus3, m) != LTC_MP_EQ) {
if ((err = mp_init_multi(&mu, &ma, NULL)) != CRYPT_OK) { goto error; }
if ((err = mp_montgomery_normalization(mu, m)) != CRYPT_OK) { goto error; }
if ((err = mp_mulmod(a, mu, m, ma)) != CRYPT_OK) { goto error; }
}
if (ltc_mp.ecc_mul2add == NULL) {
if ((err = ltc_mp.ecc_ptmul(u1, mG, mG, a, m, 0)) != CRYPT_OK) { goto error; }
if ((err = ltc_mp.ecc_ptmul(u2, mQ, mQ, a, m, 0)) != CRYPT_OK) { goto error; }
if ((err = ltc_mp.ecc_ptadd(mQ, mG, mG, ma, m, mp)) != CRYPT_OK) { goto error; }
if ((err = ltc_mp.ecc_map(mG, m, mp)) != CRYPT_OK) { goto error; }
} else {
if ((err = ltc_mp.ecc_mul2add(mG, u1, mQ, u2, mG, ma, m)) != CRYPT_OK) { goto error; }
}
if ((err = mp_mod(mG->x, p, v)) != CRYPT_OK) { goto error; }
if (mp_cmp(v, r) == LTC_MP_EQ) {
*stat = 1;
}
err = CRYPT_OK;
error:
if (mG != NULL) ltc_ecc_del_point(mG);
if (mQ != NULL) ltc_ecc_del_point(mQ);
if (mu != NULL) mp_clear(mu);
if (ma != NULL) mp_clear(ma);
mp_clear_multi(r, s, v, w, u1, u2, e, a_plus3, NULL);
if (mp != NULL) mp_montgomery_free(mp);
return err;
}
#endif
